#pragma once
#include "Singleton.hpp"
#include "utilities.hpp"

class RedisConnectPool {
 public:
  RedisConnectPool(std::size_t size, const char *host, const char *port,
                   const char *pwd = "");
  ~RedisConnectPool();

  redisContext *GetConnection();
  void ReturnConnection(redisContext *context);
  void Close();

 private:
  std::atomic<bool> stop_;
  const char *host_;
  const char *port_;
  std::size_t size_;
  std::queue<redisContext *> connections_;
  std::condition_variable cond_;
  std::mutex mtx_;
};

class RedisManager : public Singleton<RedisManager>,
                     public std::enable_shared_from_this<RedisManager> {
  friend Singleton<RedisManager>;

 public:
  ~RedisManager();
  bool Get(const std::string &key, std::string &value);
  bool Set(const std::string &key, const std::string &value);
  // 密码认证
  bool Auth(const std::string &password);
  bool LPush(const std::string &key, const std::string &value);
  bool LPop(const std::string &key, std::string &value);
  bool RPush(const std::string &key, const std::string &value);
  bool RPop(const std::string &key, std::string &value);
  bool HSet(const std::string &key, const std::string &hkey,
            const std::string &value);
  bool HSet(const char *key, const char *hkey, const char *hvalue,
            size_t hvaluelen);
  std::string HGet(const std::string &key, const std::string &hkey);
  bool Del(const std::string &key);
  bool HDel(const std::string &key, const std::string &filed);
  bool ExistsKey(const std::string &key);
  void Close();

 private:
  RedisManager();
  std::unique_ptr<RedisConnectPool> pool_;
};