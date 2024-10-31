#include "CServer.hpp"

#include "AsioIOServicePool.hpp"
#include "CSession.hpp"
#include "UserManager.hpp"

CServer::CServer(net::io_context& ioc, uint16_t port)
    : ioc_(ioc),
      acceptor_(ioc_, tcp::endpoint(net::ip::address_v4::any(), port)),
      port_(port) {
  std::cout << "Server start success, listion to port: " << port << std::endl;
  StartAccpet();
}

CServer::~CServer() {}

void CServer::ClearSession(std::string session_id) {
  if (sessions_.find(session_id) != sessions_.end()) {
    UserManager::GetInstance()->RemoveUserSession(
        sessions_[session_id]->GetUserId());
  }

  {
    std::lock_guard<std::mutex> lock(mtx_);
    sessions_.erase(session_id);
  }
}

void CServer::HandleAccept(std::shared_ptr<CSession> session,
                           const boost::system::error_code& ec) {
  if (!ec) {
    session->Start();
    std::lock_guard<std::mutex> lock(mtx_);
    sessions_[session->GetSeesionId()] = session;
  } else {
    std::cout << "Session accept failed, error is " << ec.what() << std::endl;
  }
  StartAccpet();
}

void CServer::StartAccpet() {
  auto& ioc = AsioIOServicePool::GetInstance()->GetIOService();
  std::shared_ptr<CSession> new_session = std::make_shared<CSession>(ioc, this);
  acceptor_.async_accept(new_session->GetSocket(),
                         [=](boost::system::error_code ec) {
                           this->HandleAccept(new_session, ec);
                         });
}
