#pragma once
#include "Singleton.hpp"
#include "data.hpp"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "utilities.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::AddFriendRequest;
using message::AddFriendResponse;
using message::AuthFriendRequest;
using message::AuthFriendResponse;
using message::TextChatData;
using message::TextChatMsgRequest;
using message::TextChatMsgResponse;

using message::ChatService;

class ChatConnectionPool {
 public:
  ChatConnectionPool(std::size_t size, std::string host, std::string port);
  ~ChatConnectionPool();
  std::unique_ptr<ChatService::Stub> GetConnection();
  void ReturnConnection(std::unique_ptr<ChatService::Stub> connection);
  void Close();

 private:
  std::atomic<bool> stop_;
  std::size_t size_;
  std::string host_;
  std::string port_;
  std::queue<std::unique_ptr<ChatService::Stub>> connections_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

class ChatGrpcClient : public Singleton<ChatGrpcClient> {
  friend Singleton<ChatGrpcClient>;

 public:
  ~ChatGrpcClient() {}
  AddFriendResponse NotifyAddFriend(std::string server_ip,
                                    const AddFriendRequest& request);
  AuthFriendResponse NotifyAuthFriend(std::string server_ip,
                                      const AuthFriendRequest& request);
  bool GetBaseInfo(std::string base_key, int uid,
                   std::shared_ptr<UserInfo>& userinfo);
  TextChatMsgResponse NotifyTextChatMsg(std::string server_ip,
                                        const TextChatMsgRequest& request,
                                        const Json::Value& rtvalue);

 private:
  ChatGrpcClient();
  std::unordered_map<std::string, std::unique_ptr<ChatConnectionPool>> pools_;
};