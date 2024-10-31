#include "LogicSystem.hpp"

#include "CSession.hpp"
#include "ChatGrpcClient.hpp"
#include "ConfigManager.hpp"
#include "MsgNode.hpp"
#include "MysqlManager.hpp"
#include "RedisManager.hpp"
#include "UserManager.hpp"
#include "data.hpp"

LogicSystem::LogicSystem() : stop_(false) {
  RegisterCallback();
  worker_ = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
  stop_ = true;
  cond_.notify_all();
  worker_.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) return;
  msg_que_.push(msg);
  cond_.notify_one();
}

void LogicSystem::DealMsg() {
  while (true) {
    std::unique_lock<std::mutex> lock(mtx_);
    cond_.wait(lock, [this]() { return !msg_que_.empty(); });
    if (stop_) {
      while (!msg_que_.empty()) {
        auto logic_node = msg_que_.front();
        std::cout << "Recv_msg id  is " << logic_node->recvnode_->msg_id_
                  << std::endl;
        auto callback_it = func_callbacks_.find(logic_node->recvnode_->msg_id_);
        if (callback_it == func_callbacks_.end()) {
          msg_que_.pop();
          continue;
        }
        callback_it->second(logic_node->session_,
                            logic_node->recvnode_->msg_id_,
                            std::string(logic_node->recvnode_->data_,
                                        logic_node->recvnode_->curr_len_));
        msg_que_.pop();
      }
      break;
    }
    auto logic_node = msg_que_.front();
    std::cout << "Recv_msg id  is " << logic_node->recvnode_->msg_id_
              << std::endl;
    auto callback_it = func_callbacks_.find(logic_node->recvnode_->msg_id_);
    if (callback_it == func_callbacks_.end()) {
      msg_que_.pop();
      continue;
    }
    callback_it->second(logic_node->session_, logic_node->recvnode_->msg_id_,
                        std::string(logic_node->recvnode_->data_,
                                    logic_node->recvnode_->curr_len_));
    msg_que_.pop();
  }
}

void LogicSystem::RegisterCallback() {
  func_callbacks_[MSG_CHAT_LOGIN] =
      std::bind(&LogicSystem::LoginHandler, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session,
                               const uint16_t& msg_id,
                               const std::string& msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid = root["uid"].asInt();
  auto token = root["token"].asString();
  std::cout << "user login uid is  " << uid << " user token  is " << token
            << std::endl;

  // return value
  Json::Value rv;
  Defer defer([this, &rv, session]() {
    std::string json_str = rv.toStyledString();
    session->Send(json_str, MSG_CHAT_LOGIN_RSP);
  });

  // 从redis获取用户token是否正确
  std::string token_key = kUserTokenPrefix + token;
  std::string token_value = "";
  bool success = RedisManager::GetInstance()->Get(token_key, token_value);
  if (!success) {
    rv["error"] = ErrorCodes::UidInvalid;
    return;
  }
  if (token != token_value) {
    rv["error"] = ErrorCodes::TokenInvalid;
    return;
  }
  rv["error"] = ErrorCodes::Success;

  std::string uid_str = std::to_string(uid);
  std::string base_key = kUserBaseInfo + uid_str;
  auto user_info = std::make_shared<UserInfo>();
  success = GetBaseInfo(base_key, uid, user_info);
  if (!success) {
    rv["error"] = ErrorCodes::UidInvalid;
    return;
  }
  rv["uid"] = uid;
  rv["pwd"] = user_info->pwd;
  rv["name"] = user_info->name;
  rv["email"] = user_info->email;
  rv["nick"] = user_info->nick;
  rv["desc"] = user_info->desc;
  rv["sex"] = user_info->sex;
  rv["icon"] = user_info->icon;

  // 从数据库获取申请列表
  std::vector<std::shared_ptr<ApplyInfo>> apply_list;
  auto b_apply = GetFriendApplyInfo(uid, apply_list);
  if (b_apply) {
    for (auto& apply : apply_list) {
      Json::Value obj;
      obj["name"] = apply->name;
      obj["uid"] = apply->uid;
      obj["icon"] = apply->icon;
      obj["nick"] = apply->nick;
      obj["sex"] = apply->sex;
      obj["desc"] = apply->desc;
      obj["status"] = apply->status;
      rv["apply_list"].append(obj);
    }
  }

  // 获取好友列表
  std::vector<std::shared_ptr<UserInfo>> friend_list;
  success = GetFriendList(uid, friend_list);
  for (auto& friend_ele : friend_list) {
    Json::Value obj;
    obj["name"] = friend_ele->name;
    obj["uid"] = friend_ele->uid;
    obj["icon"] = friend_ele->icon;
    obj["nick"] = friend_ele->nick;
    obj["sex"] = friend_ele->sex;
    obj["desc"] = friend_ele->desc;
    obj["back"] = friend_ele->back;
    rv["friend_list"].append(obj);
  }

  auto server_name = ConfigManager::GetInstance()["SelfServer"]["Name"];
  // 将登录数量增加
  auto rd_res = RedisManager::GetInstance()->HGet(kLoginCount, server_name);
  int count = 0;
  if (!rd_res.empty()) {
    count = std::stoi(rd_res);
  }

  count++;
  auto count_str = std::to_string(count);
  RedisManager::GetInstance()->HSet(kLoginCount, server_name, count_str);
  // session绑定用户uid
  session->SetUserId(uid);
  // 为用户设置登录ip server的名字
  std::string ipkey = kUserIpPrefix + uid_str;
  RedisManager::GetInstance()->Set(ipkey, server_name);
  // uid和session绑定管理,方便以后踢人操作
  UserManager::GetInstance()->SetUserSession(uid, session);

  return;
}

bool LogicSystem::GetBaseInfo(const std::string& base_key, int uid,
                              std::shared_ptr<UserInfo>& userinfo) {
  // 有先查redis
  std::string info_str = "";
  bool success = RedisManager::GetInstance()->Get(base_key, info_str);
  if (success) {
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
    // redis没有则查数据库
    userinfo = MysqlManager::GetInstance()->GetUser(uid);
    if (userinfo == nullptr) return false;
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

bool LogicSystem::GetFriendApplyInfo(
    int to_uid, std::vector<std::shared_ptr<ApplyInfo>>& list) {
  return MysqlManager::GetInstance()->GetApplyList(to_uid, list, 0);
}

bool LogicSystem::GetFriendList(
    int self_id, std::vector<std::shared_ptr<UserInfo>>& friend_list) {
  return MysqlManager::GetInstance()->GetFriendList(self_id, friend_list);
}

void LogicSystem::SearchInfo(std::shared_ptr<CSession> session,
                             const short& msg_id, const std::string& msg_data) {
}

void LogicSystem::AddFriendApply(std::shared_ptr<CSession> session,
                                 const short& msg_id,
                                 const std::string& msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid = root["uid"].asInt();
  auto applyname = root["applyname"].asString();
  auto bakname = root["bakname"].asString();
  auto touid = root["touid"].asInt();
  std::cout << "user login uid is  " << uid << " applyname  is " << applyname
            << " bakname is " << bakname << " touid is " << touid << std::endl;

  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  Defer defer([this, &rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_ADD_FRIEND_RSP);
  });

  // 先更新数据库
  MysqlManager::GetInstance()->AddFriendApply(uid, touid);

  // 查询redis 查找touid对应的server ip
  auto to_str = std::to_string(touid);
  auto to_ip_key = kUserIpPrefix + to_str;
  std::string to_ip_value = "";
  bool success = RedisManager::GetInstance()->Get(to_ip_key, to_ip_value);
  if (!success) {
    return;
  }

  auto& cfg = ConfigManager::GetInstance();
  auto self_name = cfg["SelfServer"]["Name"];

  std::string base_key = kUserBaseInfo + std::to_string(uid);
  auto apply_info = std::make_shared<UserInfo>();
  bool b_info = GetBaseInfo(base_key, uid, apply_info);

  // 直接通知对方有申请消息
  if (to_ip_value == self_name) {
    auto session = UserManager::GetInstance()->GetSession(touid);
    if (session) {
      // 在内存中则直接发送通知对方
      Json::Value notify;
      notify["error"] = ErrorCodes::Success;
      notify["applyuid"] = uid;
      notify["name"] = applyname;
      notify["desc"] = "";
      if (b_info) {
        notify["icon"] = apply_info->icon;
        notify["sex"] = apply_info->sex;
        notify["nick"] = apply_info->nick;
      }
      std::string return_str = notify.toStyledString();
      session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
    }

    return;
  }

  AddFriendRequest add_request;
  add_request.set_applyuid(uid);
  add_request.set_touid(touid);
  add_request.set_name(applyname);
  add_request.set_desc("");
  if (b_info) {
    add_request.set_icon(apply_info->icon);
    add_request.set_sex(apply_info->sex);
    add_request.set_nick(apply_info->nick);
  }

  // 发送通知
  ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, add_request);
}

void LogicSystem::AuthFriendApply(std::shared_ptr<CSession> session,
                                  const short& msg_id,
                                  const std::string& msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);

  auto uid = root["fromuid"].asInt();
  auto touid = root["touid"].asInt();
  auto back_name = root["back"].asString();
  std::cout << "from " << uid << " auth friend to " << touid << std::endl;

  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  auto user_info = std::make_shared<UserInfo>();

  std::string base_key = kUserBaseInfo + std::to_string(touid);
  bool b_info = GetBaseInfo(base_key, touid, user_info);
  if (b_info) {
    rtvalue["name"] = user_info->name;
    rtvalue["nick"] = user_info->nick;
    rtvalue["icon"] = user_info->icon;
    rtvalue["sex"] = user_info->sex;
    rtvalue["uid"] = touid;
  } else {
    rtvalue["error"] = ErrorCodes::UidInvalid;
  }

  Defer defer([this, &rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_AUTH_FRIEND_RSP);
  });

  // 先更新数据库
  MysqlManager::GetInstance()->AuthFriendApply(uid, touid);

  // 更新数据库添加好友
  MysqlManager::GetInstance()->AddFriend(uid, touid, back_name);

  // 查询redis 查找touid对应的server ip
  auto to_str = std::to_string(touid);
  auto to_ip_key = kUserIpPrefix + to_str;
  std::string to_ip_value = "";
  bool b_ip = RedisManager::GetInstance()->Get(to_ip_key, to_ip_value);
  if (!b_ip) {
    return;
  }

  auto& cfg = ConfigManager::GetInstance();
  auto self_name = cfg["SelfServer"]["Name"];
  // 直接通知对方有认证通过消息
  if (to_ip_value == self_name) {
    auto session = UserManager::GetInstance()->GetSession(touid);
    if (session) {
      // 在内存中则直接发送通知对方
      Json::Value notify;
      notify["error"] = ErrorCodes::Success;
      notify["fromuid"] = uid;
      notify["touid"] = touid;
      std::string base_key = kUserBaseInfo + std::to_string(uid);
      auto user_info = std::make_shared<UserInfo>();
      bool b_info = GetBaseInfo(base_key, uid, user_info);
      if (b_info) {
        notify["name"] = user_info->name;
        notify["nick"] = user_info->nick;
        notify["icon"] = user_info->icon;
        notify["sex"] = user_info->sex;
      } else {
        notify["error"] = ErrorCodes::UidInvalid;
      }

      std::string return_str = notify.toStyledString();
      session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
    }

    return;
  }

  AuthFriendRequest auth_request;
  auth_request.set_fromuid(uid);
  auth_request.set_touid(touid);

  // 发送通知
  ChatGrpcClient::GetInstance()->NotifyAuthFriend(to_ip_value, auth_request);
}

void LogicSystem::DealChatTextMsg(std::shared_ptr<CSession> session,
                                  const short& msg_id,
                                  const std::string& msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);

  int uid = root["fromid"].asInt();
  int touid = root["touid"].asInt();

  const Json::Value arrays = root["text_array"];

  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  rtvalue["text_array"] = arrays;
  rtvalue["fromuid"] = uid;
  rtvalue["touid"] = touid;

  Defer defer([this, &rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
  });

  // 查询redis 查找touid对应的server ip
  auto to_str = std::to_string(touid);
  auto to_ip_key = kUserIpPrefix + to_str;
  std::string to_ip_value = "";
  bool b_ip = RedisManager::GetInstance()->Get(to_ip_key, to_ip_value);
  if (!b_ip) {
    return;
  }

  auto& cfg = ConfigManager::GetInstance();
  auto self_name = cfg["SelfServer"]["Name"];
  // 直接通知对方有消息
  if (to_ip_value == self_name) {
    auto session = UserManager::GetInstance()->GetSession(touid);
    if (session) {
      // 在内存中则直接发送通知对方
      std::string return_str = rtvalue.toStyledString();
      session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
    }

    return;
  }

  TextChatMsgRequest text_msg_req;
  text_msg_req.set_fromuid(uid);
  text_msg_req.set_touid(touid);
  for (const auto& txt_obj : arrays) {
    auto content = txt_obj["content"].asString();
    auto msgid = txt_obj["msgid"].asString();
    std::cout << "content is " << content << std::endl;
    std::cout << "msgid is " << msgid << std::endl;
    auto* text_msg = text_msg_req.add_textmsgs();
    text_msg->set_msgid(msgid);
    text_msg->set_msgcontent(content);
  }

  // 发送通知
  ChatGrpcClient::GetInstance()->NotifyTextChatMsg(to_ip_value, text_msg_req,
                                                   rtvalue);
}

bool LogicSystem::IsPureDigit(const std::string& str) {
  for (char c : str) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return true;
}
