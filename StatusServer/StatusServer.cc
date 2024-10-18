#include "ConfigManager.hpp"
#include "RedisManager.hpp"
#include "StatusServerService.hpp"
#include "utilities.hpp"

void RunServer() {
  auto& config_manager = ConfigManager::GetInstance();
  std::string server_address(config_manager["StatusServer"]["Host"] + ":" +
                             config_manager["StatusServer"]["Port"]);
  StatusServerService service;
  grpc::ServerBuilder builder;
  // 监听端口和添加服务
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  // 构建并启动gRPC服务器
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  // 创建Boost.Asio的io_context
  boost::asio::io_context ioc;
  // 创建signal_set用于捕获SIGINT
  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
  // 设置异步等待SIGINT信号
  signals.async_wait(
      [&](const boost::system::error_code& error, int signal_number) {
        if (!error) {
          std::cout << "Shutting down server..." << std::endl;
          server->Shutdown();
          ioc.stop();
        }
      });
  // 在单独的线程中运行io_context
  std::thread([&]() { ioc.run(); }).detach();
  // 等待服务器关闭
  server->Wait();
}

int main(int argc, char* argv[]) {
  try {
    RunServer();
  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}