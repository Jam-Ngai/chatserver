#pragma once

#include "message.grpc.pb.h"
#include "message.pb.h"
#include "utilities.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerRequest;
using message::GetChatServerResponse;
using message::LoginRequest;
using message::LoginResponse;
using message::StatusService;

struct ChatServer {
  std::string host;
  std::string port;
  std::string name;
  std::size_t count;
};

class StatusServerService final : public message::StatusService::Service {
 public:
  StatusServerService();
  ~StatusServerService();
  Status GetChatServer(ServerContext* context,
                       const GetChatServerRequest* request,
                       GetChatServerResponse* response) override;
  Status Login(ServerContext* context, const LoginRequest* request,
               LoginResponse* response) override;

 private:
  void InsertToken(int uid, std::string token);
  ChatServer LeastConnection();
  std::unordered_map<std::string, ChatServer> servers_;
  std::mutex server_mtx_;
  std::unordered_map<int, std::string> tokens_;
  std::mutex token_mtx_;
};
