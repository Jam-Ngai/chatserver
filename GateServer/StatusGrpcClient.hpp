#pragma once
#include "Singleton.hpp"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "utilities.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using message::GetChatServerRequest;
using message::GetChatServerResponse;
using message::LoginRequest;
using message::LoginResponse;
using message::StatusService;

class StatusConnectPool {
 public:
  StatusConnectPool(std::size_t size, const std::string& host,
                    const std::string& port);
  ~StatusConnectPool();
  std::unique_ptr<StatusService::Stub> GetConnection();
  void ReturnConnection(std::unique_ptr<StatusService::Stub> conn);
  void Close();

 private:
  std::size_t size_;
  std::string host_;
  std::string port_;
  std::queue<std::unique_ptr<StatusService::Stub>> connections_;
  std::mutex mtx_;
  std::condition_variable cond_;
  std::atomic<bool> stop_;
};

class StatusGrpcClient : public Singleton<StatusGrpcClient> {
  friend class Singleton<StatusGrpcClient>;

 public:
  ~StatusGrpcClient() {}
  GetChatServerResponse GetChatServer(int uid);

 private:
  StatusGrpcClient();
  std::unique_ptr<StatusConnectPool> pool_;
};