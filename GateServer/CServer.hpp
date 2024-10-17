#pragma once
#include "utilities.hpp"

class CServer : public std::enable_shared_from_this<CServer> {
 public:
  CServer(net::io_context& ioc, uint16_t port);
  void Start();

 private:
  net::io_context& ioc_;
  tcp::acceptor acceptor_;
};