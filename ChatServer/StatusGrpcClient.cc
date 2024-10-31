#include "StatusGrpcClient.hpp"

#include "ConfigManager.hpp"

StatusConnectionPool::StatusConnectionPool(std::size_t size, std::string host,
                                           std::string port)
    : stop_(false), host_(host), port_(port) {
  for (std::size_t i = 0; i < size; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    connections_.push(StatusService::NewStub(channel));
  }
}
StatusConnectionPool::~StatusConnectionPool() {
  std::lock_guard<std::mutex> lock(mtx_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

void StatusConnectionPool::Close() {
  stop_ = true;
  cond_.notify_all();
}

std::unique_ptr<StatusService::Stub> StatusConnectionPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cond_.wait(lock, [this]() { return !connections_.empty(); });
  if (stop_) return nullptr;
  auto conn = std::move(connections_.front());
  connections_.pop();
  return conn;
}

void StatusConnectionPool::ReturnConnection(
    std::unique_ptr<StatusService::Stub> connection) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) return;
  connections_.push(std::move(connection));
  cond_.notify_one();
}

LoginResponse StatusGrpcClient::Login(int uid, std::string token) {
  ClientContext context;
  LoginResponse response;
  LoginRequest request;

  request.set_uid(uid);
  request.set_token(token);
  auto stub = pool_->GetConnection();
  Defer defer([this, &stub]() { pool_->ReturnConnection(std::move(stub)); });
  Status status = stub->Login(&context, request, &response);
  if (!status.ok()) {
    response.set_error(ErrorCodes::RPCFailed);
  }
  return response;
}

GetChatServerResponse StatusGrpcClient::GetChatServer(int uid) {
  ClientContext context;
  GetChatServerRequest request;
  GetChatServerResponse response;
  request.set_uid(uid);
  auto stub = pool_->GetConnection();
  Defer defer([this, &stub]() { pool_->ReturnConnection(std::move(stub)); });
  Status status = stub->GetChatServer(&context, request, &response);
  if (!status.ok()) {
    response.set_error(ErrorCodes::RPCFailed);
  }
  return response;
}

StatusGrpcClient::StatusGrpcClient() {
  auto& config_manager = ConfigManager::GetInstance();
  std::string host = config_manager["StatusServer"]["Host"];
  std::string port = config_manager["StatusServer"]["Port"];
  pool_.reset(new StatusConnectionPool(5, host, port));
}

StatusGrpcClient::~StatusGrpcClient() {}