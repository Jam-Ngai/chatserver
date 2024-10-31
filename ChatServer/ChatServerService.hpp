#pragma once
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "utilities.hpp"
#include "data.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using message::AddFriendRequest;
using message::AddFriendResponse;

using message::AuthFriendRequest;
using message::AuthFriendResponse;

using message::SendChatMsgRequest;
using message::SendChatMsgResponse;

using message::TextChatData;
using message::TextChatMsgRequest;
using message::TextChatMsgResponse;

using message::ChatService;

class ChatServerService final : public ChatService::Service {
 public:
  Status NotifyAddFriend(ServerContext* context,
                         const AddFriendRequest* request,
                         AddFriendResponse* response);
  // Status SendChatMsg(ServerContext* context, const SendChatMsgRequest*
  // request,
  //                    SendChatMsgResponse* response);
  Status NotifyAuthFriend(ServerContext* context,
                          const AuthFriendRequest* request,
                          AuthFriendResponse* response);
  Status NotifyTextChatMsg(ServerContext* context,
                           const TextChatMsgRequest* request,
                           TextChatMsgResponse* response);
  bool GetBaseInfo(std::string base_key, int uid,
                   std::shared_ptr<UserInfo>& userinfo);
};