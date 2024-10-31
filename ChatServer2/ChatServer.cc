#include "AsioIOServicePool.hpp"
#include "CServer.hpp"
#include "ChatServerService.hpp"
#include "ConfigManager.hpp"
#include "RedisManager.hpp"
#include "utilities.hpp"

int main() {
  auto& config_manager = ConfigManager::GetInstance();
  std::string server_name = config_manager["SelfServer"]["Name"];
  try {
    auto pool = AsioIOServicePool::GetInstance();

    RedisManager::GetInstance()->HSet(kLoginCount, server_name, "0");

    ChatServerService service;
    grpc::ServerBuilder builder;
    std::string grpc_addr = config_manager["SelfServer"]["Host"] + ":" +
                            config_manager["SelfServer"]["RPCPort"];
    builder.AddListeningPort(grpc_addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
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
    RedisManager::GetInstance()->HDel(kLoginCount, server_name);
    grpc_thread.join();
  } catch (std::exception& e) {
    RedisManager::GetInstance()->HDel(kLoginCount, server_name);
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}