#pragma once

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

// std
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

// third
#include <grpcpp/grpcpp.h>
#include <hiredis/hiredis.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <mysql-cppconn/jdbc/cppconn/exception.h>
#include <mysql-cppconn/jdbc/cppconn/prepared_statement.h>
#include <mysql-cppconn/jdbc/cppconn/resultset.h>
#include <mysql-cppconn/jdbc/cppconn/statement.h>
#include <mysql-cppconn/jdbc/mysql_connection.h>
#include <mysql-cppconn/jdbc/mysql_driver.h>

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
  Success = 0,
  Error_Json = 1001,      // Json解析错误
  RPCFailed = 1002,       // RPC请求错误
  VerifyExpired = 1003,   // 验证码过期
  VerifyCodeErr = 1004,   // 验证码错误
  UserExist = 1005,       // 用户已经存在
  PasswdErr = 1006,       // 密码错误
  EmailNotMatch = 1007,   // 邮箱不匹配
  PasswdUpFailed = 1008,  // 更新密码失败
  PasswdInvalid = 1009,   // 密码更新失败
  TokenInvalid = 1010,    // Token失效
  UidInvalid = 1011,      // uid无效
};

enum MSG_IDS {
  MSG_CHAT_LOGIN = 1005,               // 用户登陆
  MSG_CHAT_LOGIN_RSP = 1006,           // 用户登陆回包
  ID_SEARCH_USER_REQ = 1007,           // 用户搜索请求
  ID_SEARCH_USER_RSP = 1008,           // 搜索用户回包
  ID_ADD_FRIEND_REQ = 1009,            // 申请添加好友请求
  ID_ADD_FRIEND_RSP = 1010,            // 申请添加好友回复
  ID_NOTIFY_ADD_FRIEND_REQ = 1011,     // 通知用户添加好友申请
  ID_AUTH_FRIEND_REQ = 1013,           // 认证好友请求
  ID_AUTH_FRIEND_RSP = 1014,           // 认证好友回复
  ID_NOTIFY_AUTH_FRIEND_REQ = 1015,    // 通知用户认证好友申请
  ID_TEXT_CHAT_MSG_REQ = 1017,         // 文本聊天信息请求
  ID_TEXT_CHAT_MSG_RSP = 1018,         // 文本聊天信息回复
  ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019,  // 通知用户文本聊天信息
};

const std::string kCodePrefix = "code_";
const std::string kUserTokenPrefix = "utoken_";
const std::string kUserIpPrefix = "uip_";
const std::string kIpCountPrefix = "ipcount_";
const std::string kUserBaseInfo = "ubaseinfo_";
const std::string kLoginCount = "logincount";
const std::string kNameInfo = "nameinfo_";

const int kMaxLength = 2048;
const int kHeadTotalLen = 4;
const int kHeadIdLen = 2;
const int kHeadDataLen = 2;
const int kMaxRecvQue = 10000;
const int kMaxSendQue = 1000;

class Defer {
 public:
  Defer(std::function<void()> func) : func_(func) {};
  ~Defer() { func_(); }

 private:
  std::function<void()> func_;
};