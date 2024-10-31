#include "ChatGrpcClient.hpp"

#include "ConfigManager.hpp"
#include "MysqlManager.hpp"
#include "RedisManager.hpp"

ChatConnectionPool::ChatConnectionPool(std::size_t size, std::string host,
                                       std::string port)
    : size_(size), host_(host), port_(port), stop_(false) {
  for (std::size_t i = 0; i < size; ++i) {
    auto channel = grpc::CreateChannel(host + ":" + port,
                                       grpc::InsecureChannelCredentials());
    connections_.push(ChatService::NewStub(channel));
  }
}

ChatConnectionPool::~ChatConnectionPool() {
  std::lock_guard<std::mutex> lock(mtx_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

void ChatConnectionPool::Close() {
  stop_ = true;
  cond_.notify_all();
}

std::unique_ptr<ChatService::Stub> ChatConnectionPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cond_.wait(lock, [this]() { return !connections_.empty(); });
  if (stop_) return nullptr;
  auto conn = std::move(connections_.front());
  connections_.pop();
  return conn;
}

void ChatConnectionPool::ReturnConnection(
    std::unique_ptr<ChatService::Stub> connection) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) return;
  connections_.push(std::move(connection));
  cond_.notify_one();
}

ChatGrpcClient::ChatGrpcClient() {
  auto& cfg = ConfigManager::GetInstance();
  auto server_list = cfg["PeerServer"]["Servers"];

  std::vector<std::string> words;

  std::stringstream ss(server_list);
  std::string word;

  while (std::getline(ss, word, ',')) {
    words.push_back(word);
  }

  for (auto& word : words) {
    if (cfg[word]["Name"].empty()) {
      continue;
    }
    pools_[cfg[word]["Name"]] = std::make_unique<ChatConnectionPool>(
        5, cfg[word]["Host"], cfg[word]["Port"]);
  }
}

AddFriendResponse ChatGrpcClient::NotifyAddFriend(
    std::string server_ip, const AddFriendRequest& request) {
  AddFriendResponse response;
  Defer defer([&response, &request]() {
    response.set_error(ErrorCodes::Success);
    response.set_applyuid(request.applyuid());
    response.set_touid(request.touid());
  });

  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return response;
  }

  auto& pool = find_iter->second;
  ClientContext context;
  auto stub = pool->GetConnection();
  Status status = stub->NotifyAddFriend(&context, request, &response);
  Defer defercon(
      [&stub, this, &pool]() { pool->ReturnConnection(std::move(stub)); });

  if (!status.ok()) {
    response.set_error(ErrorCodes::RPCFailed);
    return response;
  }

  return response;
}

AuthFriendResponse ChatGrpcClient::NotifyAuthFriend(
    std::string server_ip, const AuthFriendRequest& request) {
  AuthFriendResponse response;
  response.set_error(ErrorCodes::Success);

  Defer defer([&response, &request]() {
    response.set_fromuid(request.fromuid());
    response.set_touid(request.touid());
  });

  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return response;
  }

  auto& pool = find_iter->second;
  ClientContext context;
  auto stub = pool->GetConnection();
  Status status = stub->NotifyAuthFriend(&context, request, &response);
  Defer defercon(
      [&stub, this, &pool]() { pool->ReturnConnection(std::move(stub)); });

  if (!status.ok()) {
    response.set_error(ErrorCodes::RPCFailed);
    return response;
  }

  return response;
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid,
                                 std::shared_ptr<UserInfo>& userinfo) {
  // 优先查redis中查询用户信息
  std::string info_str = "";
  bool b_base = RedisManager::GetInstance()->Get(base_key, info_str);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info_str, root);
    userinfo->uid = root["uid"].asInt();
    userinfo->name = root["name"].asString();
    userinfo->pwd = root["pwd"].asString();
    userinfo->email = root["email"].asString();
    userinfo->nick = root["nick"].asString();
    userinfo->desc = root["desc"].asString();
    userinfo->sex = root["sex"].asInt();
    userinfo->icon = root["icon"].asString();
    std::cout << "user login uid is  " << userinfo->uid << " name  is "
              << userinfo->name << " pwd is " << userinfo->pwd << " email is "
              << userinfo->email << std::endl;
  } else {
    // redis中没有则查询mysql
    // 查询数据库
    std::shared_ptr<UserInfo> user_info = nullptr;
    user_info = MysqlManager::GetInstance()->GetUser(uid);
    if (user_info == nullptr) {
      return false;
    }

    userinfo = user_info;

    // 将数据库内容写入redis缓存
    Json::Value redis_root;
    redis_root["uid"] = uid;
    redis_root["pwd"] = userinfo->pwd;
    redis_root["name"] = userinfo->name;
    redis_root["email"] = userinfo->email;
    redis_root["nick"] = userinfo->nick;
    redis_root["desc"] = userinfo->desc;
    redis_root["sex"] = userinfo->sex;
    redis_root["icon"] = userinfo->icon;
    RedisManager::GetInstance()->Set(base_key, redis_root.toStyledString());
  }

  return true;
}

TextChatMsgResponse ChatGrpcClient::NotifyTextChatMsg(
    std::string server_ip, const TextChatMsgRequest& request,
    const Json::Value& rtvalue) {
  TextChatMsgResponse response;
  response.set_error(ErrorCodes::Success);

  Defer defer([&response, &request]() {
    response.set_touid(request.touid());
    response.set_fromuid(request.fromuid());
    for (const TextChatData& text_data : request.textmsgs()) {
      TextChatData* new_msg = response.add_textmsgs();
      new_msg->set_msgid(text_data.msgid());
      new_msg->set_msgcontent(text_data.msgcontent());
    }
  });

  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return response;
  }

  auto& pool = find_iter->second;
  ClientContext context;
  auto stub = pool->GetConnection();
  Status status = stub->NotifyTextChatMsg(&context, request, &response);
  Defer defercon(
      [&stub, this, &pool]() { pool->ReturnConnection(std::move(stub)); });

  if (!status.ok()) {
    response.set_error(ErrorCodes::RPCFailed);
    return response;
  }

  return response;
}
