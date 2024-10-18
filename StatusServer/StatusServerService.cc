#include "StatusServerService.hpp"

#include "ConfigManager.hpp"

namespace {
std::string GenerateUniqueString() {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string unique_string = boost::uuids::to_string(uuid);
  return unique_string;
}
}  // namespace

void StatusServerService::InsertToken(int uid, std::string token) {
  std::lock_guard<std::mutex> lock(token_mtx_);
  tokens_[uid] = token;
}

ChatServer StatusServerService::LeastConnection() {
  std::lock_guard<std::mutex> lock(server_mtx_);
  auto min_server = servers_.begin()->second;
  for (const auto& server : servers_) {
    if (server.second.count < min_server.count) {
      min_server = server.second;
    }
  }
  return min_server;
}

StatusServerService::StatusServerService() {
  auto& config_manager = ConfigManager::GetInstance();
  ChatServer server;
  server.port = config_manager["ChatServer1"]["Port"];
  server.host = config_manager["ChatServer1"]["Host"];
  server.name = config_manager["ChatServer1"]["Name"];
  servers_["name"] = server;

  server.port = config_manager["ChatServer2"]["Port"];
  server.host = config_manager["ChatServer2"]["Host"];
  server.name = config_manager["ChatServer2"]["Name"];
  servers_["name"] = server;
}

StatusServerService::~StatusServerService() {}

Status StatusServerService::GetChatServer(ServerContext* context,
                                    const GetChatServerRequest* request,
                                    GetChatServerResponse* response) {
  std::string prefix("Status Server has received: ");
  const ChatServer& server = LeastConnection();
  response->set_host(server.host);
  response->set_port(server.port);
  response->set_error(ErrorCodes::Success);
  response->set_token(GenerateUniqueString());
  InsertToken(request->uid(), response->token());
  return Status::OK;
}

Status StatusServerService::Login(ServerContext* context, const LoginRequest* request,
                            LoginResponse* response) {}