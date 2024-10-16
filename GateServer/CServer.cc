#include "CServer.hpp"

#include "HttpConnection.hpp"

CServer::CServer(net::io_context& ioc, uint16_t port)
    : ioc_(ioc),
      acceptor_(ioc_, tcp::endpoint(net::ip::address_v4::any(), port)),
      socket_(ioc_) {}

void CServer::Start() {
  auto self = shared_from_this();
  acceptor_.async_accept(socket_, [=](boost::system::error_code e) {
    try {
      // 出错就放弃socket连接,继续监听其他连接
      if (e) {
        self->Start();
        return;
      }
      // 创建新连接
      // 用HttpConnection类管理
      std::make_shared<HttpConnection>(std::move(self->socket_))->Start();
      self->Start();
    } catch (std::exception& e) {
      std::cout << "Exception: " << e.what() << std::endl;
    }
  });
}
