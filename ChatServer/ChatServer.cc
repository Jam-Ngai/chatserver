#include "AsioIOServicePool.hpp"
#include "CServer.hpp"
#include "ChatServerService.hpp"
#include "ConfigManager.hpp"
#include "utilities.hpp"

int main() {
  try {
    auto& config_manager = ConfigManager::GetInstance();
    auto pool = AsioIOServicePool::GetInstance();
    
    ChatServerService service;
    grpc::ServerBuilder builder;
    std::string grpc_addr = "[::]:" + config_manager["SelfServer"]["Port"];
    builder.AddListeningPort(grpc_addr, grpc::InsecureServerCredentials());
    std::unique_ptr<grpc::Server> grpc_server(builder.BuildAndStart());
    std::thread grpc_thread([&grpc_server]() { grpc_server->Wait(); });

    boost::asio::io_context ioc;
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioc, pool, &grpc_server](boost::system::error_code, int) {
          ioc.stop();
          pool->Stop();
          grpc_server->Shutdown();
        });
    std::string port = config_manager["SelfServer"]["Port"];
    CServer server(ioc, atoi(port.c_str()));
    ioc.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}