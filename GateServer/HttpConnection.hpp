#pragma once

#include "utilities.hpp"

class LogicSystem;
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
  friend class LogicSystem;

 public:
  HttpConnection(net::io_context& ioc);
  void Start();
  tcp::socket& socket() { return socket_; }

 private:
  void CheckDeadline();
  void WriteResponse();
  void HandleRequest();
  void PreParseGetParam();

  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};
  http::request<http::dynamic_body> request_;
  http::response<http::dynamic_body> response_;
  net::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};

  std::string get_url_;
  std::unordered_map<std::string, std::string> get_params_;
};