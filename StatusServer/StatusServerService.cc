#include "StatusServerService.hpp"

#include "ConfigManager.hpp"
#include "RedisManager.hpp"

namespace {
std::string GenerateUniqueString() {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string unique_string = boost::uuids::to_string(uuid);
  return unique_string;
}
}  // namespace

void StatusServerService::InsertToken(int uid, std::string token) {
  std::string uid_str = std::to_string(uid);
  std::string token_key = kUserTokenPrefix + uid_str;
  RedisManager::GetInstance()->Set(token_key, token);
}

ChatServer StatusServerService::LeastConnection() {
  std::lock_guard<std::mutex> lock(server_mtx_);
  auto min_server = servers_.begin()->second;
  int min_count;
  std::string count_str =
      RedisManager::GetInstance()->HGet(kLoginCount, min_server.name_);
  if (count_str.empty()) {
    min_count = INT_MAX;
  } else {
    min_count = std::atoi(count_str.c_str());
  }

  for (const auto& server : servers_) {
    count_str =
        RedisManager::GetInstance()->HGet(kLoginCount,server.second.name_);
    if (count_str.empty()) continue;
    if (std::atoi(count_str.c_str()) < min_count) {
      min_server = server.second;
    }
  }
  return min_server;
}

ChatServer::ChatServer() : host_(""), port_(""), name_("") {}

ChatServer::ChatServer(const ChatServer& cs)
    : host_(cs.host_), port_(cs.port_), name_(cs.name_) {}

ChatServer& ChatServer::operator=(const ChatServer& cs) {
  if (&cs == this) {
    return *this;
  }

  host_ = cs.host_;
  name_ = cs.name_;
  port_ = cs.port_;
  return *this;
}

StatusServerService::StatusServerService() {
  auto& config_manager = ConfigManager::GetInstance();

  std::string server_list = config_manager["ChatServers"]["Name"];
  std::istringstream is(server_list);
  std::string word;
  while (std::getline(is, word, ',')) {
    if (config_manager[word]["Name"].empty()) continue;
    ChatServer server;
    server.host_ = config_manager[word]["Host"];
    server.port_ = config_manager[word]["Port"];
    server.name_ = config_manager[word]["Name"];
    servers_[server.name_] = server;
  }
}

StatusServerService::~StatusServerService() {}

Status StatusServerService::GetChatServer(ServerContext* context,
                                          const GetChatServerRequest* request,
                                          GetChatServerResponse* response) {
  std::string prefix("Status Server has received: ");
  const ChatServer& server = LeastConnection();
  response->set_host(server.host_);
  response->set_port(server.port_);
  response->set_error(ErrorCodes::Success);
  response->set_token(GenerateUniqueString());
  InsertToken(request->uid(), response->token());
  return Status::OK;
}