#pragma once
#include "Singleton.hpp"
#include "message.grpc.pb.h"
#include "utilities.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using message::VerifyRequst;
using message::VerifyResponse;
using message::VerifyService;

class RpcConnectPool {
 public:
  RpcConnectPool(std::size_t size, std::string host, std::string port);
  ~RpcConnectPool();
  void Close();
  std::unique_ptr<VerifyService::Stub> GetConnection();
  void ReturnConnection(std::unique_ptr<VerifyService::Stub> context);

 private:
  std::atomic<bool> stop_;
  std::size_t size_;
  std::string host_;
  std::string port_;
  std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
  std::condition_variable cond_;
  std::mutex mtx_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
  friend class Singleton<VerifyGrpcClient>;

 public:
  VerifyResponse GetVerifyCode(std::string email);

 private:
  VerifyGrpcClient();
  std::unique_ptr<RpcConnectPool> pool_;
};