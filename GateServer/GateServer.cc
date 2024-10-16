#include "CServer.hpp"
#include "utilities.hpp"
#include "ConfigManager.hpp"

int main(int argc, char* argv[]) {
  try {
    ConfigManager kConfigManager;
    uint16_t gate_port=stoi(kConfigManager["GateServer"]["Port"]);
    uint16_t port = 8080;
    net::io_context ioc{1};
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code ec, int) {
      if (ec) {
        return;
      }
      ioc.stop();
    });
    std::make_shared<CServer>(ioc, port)->Start();
    ioc.run();
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}