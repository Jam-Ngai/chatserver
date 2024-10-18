#include "RedisManager.hpp"

#include "ConfigManager.hpp"

RedisConnectPool::RedisConnectPool(std::size_t size, const char *host,
                                   const char *port, const char *pwd)
    : size_(size), host_(host), port_(port), stop_(false) {
  for (std::size_t i = 0; i < size; ++i) {
    redisContext *context = redisConnect(host, atoi(port));
    if (context == nullptr || context->err != 0) {
      if (context != nullptr) {
        redisFree(context);
      }
      continue;
    }
    if (pwd != "") {
      auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd);
      if (reply->type == REDIS_REPLY_ERROR) {
        std::cout << "Authenticate failed!" << std::endl;
        freeReplyObject(reply);
        continue;
      }
      freeReplyObject(reply);
      std::cout << "Authenticate succeed!" << std::endl;
    }
    connections_.push(context);
  }
  std::cout << "Redis connection connected" << std::endl;
}

RedisConnectPool::~RedisConnectPool() {
  std::lock_guard<std::mutex> lock(mtx_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

redisContext *RedisConnectPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cond_.wait(lock, [this]() {
    if (stop_) {
      return true;
    }
    return !connections_.empty();
  });

  if (stop_) {
    return nullptr;
  }

  redisContext *context = connections_.front();
  connections_.pop();
  return context;
}

void RedisConnectPool::ReturnConnection(redisContext *context) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) {
    return;
  }

  connections_.push(context);
  cond_.notify_one();
}

void RedisConnectPool::Close() {
  stop_ = true;
  cond_.notify_all();
}

RedisManager::RedisManager() {
  auto &config_mannager = ConfigManager::GetInstance();
  std::string host = config_mannager["Redis"]["Host"];
  std::string port = config_mannager["Redis"]["Port"];
  pool_.reset(new RedisConnectPool(5, host.c_str(), port.c_str()));
}

RedisManager::~RedisManager() { Close(); }

void RedisManager::Close() { pool_->Close(); }

bool RedisManager::Get(const std::string &key, std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "GET %s", key.c_str());
  if (nullptr == reply) {
    std::cout << "[ GET  " << key << " ] failed" << std::endl;
    freeReplyObject(reply);
    return false;
  }

  if (reply->type != REDIS_REPLY_STRING) {
    std::cout << "[ GET  " << key << " ] failed" << std::endl;
    freeReplyObject(reply);
    return false;
  }

  value = reply->str;
  freeReplyObject(reply);

  std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
  return true;
}

bool RedisManager::Set(const std::string &key, const std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  // 执行redis命令行
  auto reply = (redisReply *)redisCommand(connect, "SET %s %s", key.c_str(),
                                          value.c_str());
  // 如果返回nullptr则说明执行失败
  if (nullptr == reply) {
    std::cout << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  // 如果执行失败则释放连接
  if (!(reply->type == REDIS_REPLY_STATUS &&
        (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
    std::cout << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
  freeReplyObject(reply);
  std::cout << "Execut command [ SET " << key << "  " << value
            << " ] success ! " << std::endl;
  return true;
}

bool RedisManager::Auth(const std::string &password) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "AUTH %s", password.c_str());
  if (reply->type == REDIS_REPLY_ERROR) {
    std::cout << "认证失败" << std::endl;
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    return false;
  } else {
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "认证成功" << std::endl;
    return true;
  }
}

bool RedisManager::LPush(const std::string &key, const std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "LPUSH %s %s", key.c_str(),
                                          value.c_str());
  if (nullptr == reply) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }

  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }

  std::cout << "Execut command [ LPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisManager::LPop(const std::string &key, std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "LPOP %s ", key.c_str());
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ LPOP " << key << " ] failure ! "
              << std::endl;
    freeReplyObject(reply);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisManager::RPush(const std::string &key, const std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "RPUSH %s %s", key.c_str(),
                                          value.c_str());
  if (nullptr == reply) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }

  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }

  std::cout << "Execut command [ RPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisManager::RPop(const std::string &key, std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "RPOP %s ", key.c_str());
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ RPOP " << key << " ] failure ! "
              << std::endl;
    freeReplyObject(reply);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisManager::HSet(const std::string &key, const std::string &hkey,
                        const std::string &value) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "HSET %s %s %s", key.c_str(),
                                          hkey.c_str(), value.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisManager::HSet(const char *key, const char *hkey, const char *hvalue,
                        size_t hvaluelen) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  const char *argv[4];
  size_t argvlen[4];
  argv[0] = "HSET";
  argvlen[0] = 4;
  argv[1] = key;
  argvlen[1] = strlen(key);
  argv[2] = hkey;
  argvlen[2] = strlen(hkey);
  argv[3] = hvalue;
  argvlen[3] = hvaluelen;
  auto reply = (redisReply *)redisCommandArgv(connect, 4, argv, argvlen);
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

std::string RedisManager::HGet(const std::string &key,
                               const std::string &hkey) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return "";
  }
  const char *argv[3];
  size_t argvlen[3];
  argv[0] = "HGET";
  argvlen[0] = 4;
  argv[1] = key.c_str();
  argvlen[1] = key.length();
  argv[2] = hkey.c_str();
  argvlen[2] = hkey.length();
  auto reply = (redisReply *)redisCommandArgv(connect, 3, argv, argvlen);
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey
              << "  ] failure ! " << std::endl;
    return "";
  }

  std::string value = reply->str;
  freeReplyObject(reply);
  std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! "
            << std::endl;
  return value;
}

bool RedisManager::Del(const std::string &key) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "DEL %s", key.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  return true;
}

bool RedisManager::ExistsKey(const std::string &key) {
  auto *connect = pool_->GetConnection();
  if (nullptr == connect) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "exists %s", key.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ||
      reply->integer == 0) {
    std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
    freeReplyObject(reply);
    return false;
  }
  std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
  freeReplyObject(reply);
  return true;
}
