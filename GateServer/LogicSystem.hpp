#pragma once

#include "Singleton.hpp"
#include "utilities.hpp"

class HttpConnection;

using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

 public:
  ~LogicSystem() {};

  bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
  bool HandlePost(std::string, std::shared_ptr<HttpConnection>);
  void RegisterGet(std::string, HttpHandler handler);
  void RegisterPost(std::string, HttpHandler handler);

 private:
  LogicSystem();
  std::unordered_map<std::string, HttpHandler> get_handlers_;
  std::unordered_map<std::string, HttpHandler> post_handlers_;
};