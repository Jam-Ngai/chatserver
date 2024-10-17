#include "VerifyGrpcClient.hpp"

#include "ConfigManager.hpp"

RpcConnectPool::RpcConnectPool(std::size_t size, std::string host,
                               std::string port)
    : size_(size), host_(host), port_(port), stop_(false) {
  for (std::size_t i = 0; i < size_; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    connections_.push(VerifyService::NewStub(channel));
  }
}

RpcConnectPool::~RpcConnectPool() {
  std::lock_guard<std::mutex> lock(mtx_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

void RpcConnectPool::Close() {
  stop_ = true;
  cond_.notify_all();
}

std::unique_ptr<VerifyService::Stub> RpcConnectPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cond_.wait(lock, [this]() {
    if (this->stop_) {
      return true;
    }
    return !this->connections_.empty();
  });
  if (stop_) {
    return nullptr;
  }
  auto context = std::move(connections_.front());
  connections_.pop();
  return context;
}

void RpcConnectPool::ReturnConnection(
    std::unique_ptr<VerifyService::Stub> context) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) {
    return;
  }
  connections_.push(std::move(context));
  cond_.notify_one();
}

VerifyGrpcClient::VerifyGrpcClient() {
  auto& config_manager = ConfigManager::GetInstance();
  std::string host = config_manager["VerifyServer"]["Host"];
  std::string port = config_manager["VerifyServer"]["Port"];
  pool_.reset(new RpcConnectPool(5, host, port));
}

VerifyResponse VerifyGrpcClient::GetVerifyCode(std::string email) {
  ClientContext context;
  VerifyResponse reply;
  VerifyRequst request;
  request.set_email(email);
  auto stub = pool_->GetConnection();
  Status state = stub->GetVerifyCode(&context, request, &reply);
  if (state.ok()) {
    pool_->ReturnConnection(std::move(stub));
    return reply;
  } else {
    pool_->ReturnConnection(std::move(stub));
    reply.set_error(ErrorCodes::RPCFailed);
    return reply;
  }
}