#pragma once
#include "utilities.hpp"
class CSession;

class CServer : public std::enable_shared_from_this<CServer> {
 public:
  CServer(net::io_context& ioc, uint16_t port);
  ~CServer();
  void ClearSession(std::string session_id);

 private:
  void HandleAccept(std::shared_ptr<CSession> session,
                    const boost::system::error_code& ec);
  void StartAccpet();

  net::io_context& ioc_;
  tcp::acceptor acceptor_;
  uint16_t port_;
  std::unordered_map<std::string, std::shared_ptr<CSession>> sessions_;
  std::mutex mtx_;
};