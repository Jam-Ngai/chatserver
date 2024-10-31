#include "CServer.hpp"
#include "ConfigManager.hpp"
#include "utilities.hpp"
#include "RedisManager.hpp"

int main(int argc, char* argv[]) {
  try {
    ConfigManager& config_manager = ConfigManager::GetInstance();
    uint16_t gate_port = stoi(config_manager["GateServer"]["Port"]);
    net::io_context ioc{1};
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code &ec, int) {
      if (ec) {
        return;
      }
      ioc.stop();
    });
    std::make_shared<CServer>(ioc, gate_port)->Start();
    ioc.run();
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}