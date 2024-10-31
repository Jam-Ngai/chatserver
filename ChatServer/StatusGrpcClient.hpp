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

class StatusConnectionPool {
 public:
  StatusConnectionPool(std::size_t size, std::string host, std::string port);
  ~StatusConnectionPool();
  std::unique_ptr<StatusService::Stub> GetConnection();
  void ReturnConnection(std::unique_ptr<StatusService::Stub> connection);
  void Close();

 private:
  std::atomic<bool> stop_;
  std::size_t size_;
  std::string host_;
  std::string port_;
  std::queue<std::unique_ptr<StatusService::Stub>> connections_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

class StatusGrpcClient : public Singleton<StatusGrpcClient> {
  friend class Singleton<StatusGrpcClient>;

 public:
  ~StatusGrpcClient();
  LoginResponse Login(int uid, std::string token);
  GetChatServerResponse GetChatServer(int uid);

 private:
  StatusGrpcClient();
  std::unique_ptr<StatusConnectionPool> pool_;
};