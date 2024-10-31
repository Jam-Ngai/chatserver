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

class ChatServer {
 public:
  ChatServer();
  ChatServer(const ChatServer&);
  ChatServer& operator=(const ChatServer&);
  std::string host_;
  std::string port_;
  std::string name_;
};

class StatusServerService final : public message::StatusService::Service {
 public:
  StatusServerService();
  ~StatusServerService();
  Status GetChatServer(ServerContext* context,
                       const GetChatServerRequest* request,
                       GetChatServerResponse* response) override;

 private:
  void InsertToken(int uid, std::string token);
  ChatServer LeastConnection();
  std::unordered_map<std::string, ChatServer> servers_;
  std::mutex server_mtx_;
};
