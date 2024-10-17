#include "CServer.hpp"

#include "AsioIOServicePool.hpp"
#include "HttpConnection.hpp"

CServer::CServer(net::io_context& ioc, uint16_t port)
    : ioc_(ioc),
      acceptor_(ioc_, tcp::endpoint(net::ip::address_v4::any(), port)) {}

void CServer::Start() {
  auto self = shared_from_this();
  auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
  // 创建新连接
  // 用HttpConnection类管理
  auto new_connection = std::make_shared<HttpConnection>(io_context);
  acceptor_.async_accept(
      new_connection->socket(), [=](boost::system::error_code e) {
        try {
          // 出错就放弃socket连接,继续监听其他连接
          if (e) {
            self->Start();
            return;
          }
          // 启动链接
          new_connection->Start();
          // 继续监听新连接
          self->Start();
        } catch (std::exception& e) {
          std::cout << "Exception: " << e.what() << std::endl;
        }
      });
}
