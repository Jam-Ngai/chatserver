#include "ChatServerService.hpp"

#include "CSession.hpp"
#include "MysqlManager.hpp"
#include "RedisManager.hpp"
#include "UserManager.hpp"

Status ChatServerService::NotifyAddFriend(ServerContext* context,
                                          const AddFriendRequest* request,
                                          AddFriendResponse* response) {
  // 查找用户是否在本服务器
  auto touid = request->touid();
  auto session = UserManager::GetInstance()->GetSession(touid);

  Defer defer([request, response]() {
    response->set_error(ErrorCodes::Success);
    response->set_applyuid(request->applyuid());
    response->set_touid(request->touid());
  });

  // 在内存中则直接发送通知对方
  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  rtvalue["applyuid"] = request->applyuid();
  rtvalue["name"] = request->name();
  rtvalue["desc"] = request->desc();
  rtvalue["icon"] = request->icon();
  rtvalue["sex"] = request->sex();
  rtvalue["nick"] = request->nick();

  std::string return_str = rtvalue.toStyledString();

  session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
  return Status::OK;
}

Status ChatServerService::NotifyAuthFriend(ServerContext* context,
                                           const AuthFriendRequest* request,
                                           AuthFriendResponse* response) {
  // 查找用户是否在本服务器
  auto touid = request->touid();
  auto fromuid = request->fromuid();
  auto session = UserManager::GetInstance()->GetSession(touid);

  Defer defer([request, response]() {
    response->set_error(ErrorCodes::Success);
    response->set_fromuid(request->fromuid());
    response->set_touid(request->touid());
  });

  // 用户不在内存中则直接返回
  if (session == nullptr) {
    return Status::OK;
  }

  // 在内存中则直接发送通知对方
  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  rtvalue["fromuid"] = request->fromuid();
  rtvalue["touid"] = request->touid();

  std::string base_key = kUserBaseInfo + std::to_string(fromuid);
  auto user_info = std::make_shared<UserInfo>();
  bool b_info = GetBaseInfo(base_key, fromuid, user_info);
  if (b_info) {
    rtvalue["name"] = user_info->name;
    rtvalue["nick"] = user_info->nick;
    rtvalue["icon"] = user_info->icon;
    rtvalue["sex"] = user_info->sex;
  } else {
    rtvalue["error"] = ErrorCodes::UidInvalid;
  }

  std::string return_str = rtvalue.toStyledString();

  session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
  return Status::OK;
}

Status ChatServerService::NotifyTextChatMsg(ServerContext* context,
                                            const TextChatMsgRequest* request,
                                            TextChatMsgResponse* response) {
  // 查找用户是否在本服务器
  auto touid = request->touid();
  auto session = UserManager::GetInstance()->GetSession(touid);
  response->set_error(ErrorCodes::Success);

  // 用户不在内存中则直接返回
  if (session == nullptr) {
    return Status::OK;
  }

  // 在内存中则直接发送通知对方
  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  rtvalue["fromuid"] = request->fromuid();
  rtvalue["touid"] = request->touid();

  // 将聊天数据组织为数组
  Json::Value text_array;
  for (auto& msg : request->textmsgs()) {
    Json::Value element;
    element["content"] = msg.msgcontent();
    element["msgid"] = msg.msgid();
    text_array.append(element);
  }
  rtvalue["text_array"] = text_array;

  std::string return_str = rtvalue.toStyledString();

  session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
  return Status::OK;
}

bool ChatServerService::GetBaseInfo(std::string base_key, int uid,
                                    std::shared_ptr<UserInfo>& userinfo) {
  // 优先查redis中查询用户信息
  std::string info_str = "";
  bool b_base = RedisManager::GetInstance()->Get(base_key, info_str);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info_str, root);
    userinfo->uid = root["uid"].asInt();
    userinfo->name = root["name"].asString();
    userinfo->pwd = root["pwd"].asString();
    userinfo->email = root["email"].asString();
    userinfo->nick = root["nick"].asString();
    userinfo->desc = root["desc"].asString();
    userinfo->sex = root["sex"].asInt();
    userinfo->icon = root["icon"].asString();
    std::cout << "user login uid is  " << userinfo->uid << " name  is "
              << userinfo->name << " pwd is " << userinfo->pwd << " email is "
              << userinfo->email << std::endl;
  } else {
    // redis中没有则查询mysql
    // 查询数据库
    std::shared_ptr<UserInfo> user_info = nullptr;
    user_info = MysqlManager::GetInstance()->GetUser(uid);
    if (user_info == nullptr) {
      return false;
    }

    userinfo = user_info;

    // 将数据库内容写入redis缓存
    Json::Value redis_root;
    redis_root["uid"] = uid;
    redis_root["pwd"] = userinfo->pwd;
    redis_root["name"] = userinfo->name;
    redis_root["email"] = userinfo->email;
    redis_root["nick"] = userinfo->nick;
    redis_root["desc"] = userinfo->desc;
    redis_root["sex"] = userinfo->sex;
    redis_root["icon"] = userinfo->icon;
    RedisManager::GetInstance()->Set(base_key, redis_root.toStyledString());
  }

  return true;
}