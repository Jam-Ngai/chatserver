#pragma once

// boost
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

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

const std::string kCodePrefix = "code_";

class Defer {
 public:
  Defer(std::function<void()> func) : func_(func) {};
  ~Defer() { func_(); }

 private:
  std::function<void()> func_;
};