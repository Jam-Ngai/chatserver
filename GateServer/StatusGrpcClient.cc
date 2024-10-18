#include "StatusGrpcClient.hpp"

#include "ConfigManager.hpp"

StatusConnectPool::StatusConnectPool(std::size_t size, const std::string& host,
                                     const std::string& port)
    : size_(size), host_(host), port_(port), stop_(false) {
  for (std::size_t i = 0; i < size; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    connections_.push(StatusService::NewStub(channel));
  }
}

StatusConnectPool::~StatusConnectPool() {
  std::lock_guard<std::mutex> lock(mtx_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

std::unique_ptr<StatusService::Stub> StatusConnectPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cond_.wait(lock, [this]() {
    if (stop_) {
      return true;
    }
    return !connections_.empty();
  });
  if (stop_) {
    return nullptr;
  }
  auto conn = std::move(connections_.front());
  connections_.pop();
  return conn;
}

void StatusConnectPool::ReturnConnection(
    std::unique_ptr<StatusService::Stub> conn) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) return;
  connections_.push(std::move(conn));
  cond_.notify_one();
}

void StatusConnectPool::Close() {
  stop_ = true;
  cond_.notify_all();
}

StatusGrpcClient::StatusGrpcClient() {
  auto& config_manager = ConfigManager::GetInstance();
  std::string host = config_manager["StatusServer"]["Host"];
  std::string port = config_manager["StatusServer"]["Port"];
  pool_.reset(new StatusConnectPool(5, host, port));
}

GetChatServerResponse StatusGrpcClient::GetChatServer(int uid) {
  ClientContext context;
  GetChatServerRequest request;
  GetChatServerResponse response;
  request.set_uid(uid);
  auto stub = pool_->GetConnection();
  Status status = stub->GetChatServer(&context, request, &response);
  Defer defer([this, &stub]() { pool_->ReturnConnection(std::move(stub)); });
  if (!status.ok()) {
    response.set_error(ErrorCodes::RPCFailed);
  }
  return response;
}