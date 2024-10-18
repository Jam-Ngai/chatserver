#pragma once

#include "utilities.hpp"

class SqlConnection {
 public:
  SqlConnection(sql::Connection* conn, int64_t last_time)
      : conn_(conn), last_time_(last_time) {}
  std::unique_ptr<sql::Connection> conn_;
  int64_t last_time_;
};

class MysqlPool {
 public:
  MysqlPool(const std::string& url, const std::string& user,
            const std::string& pwd, const std::string& schema, int size);
  ~MysqlPool();

  std::unique_ptr<SqlConnection> GetConnection();
  void ReturnConnection(std::unique_ptr<SqlConnection> conn);
  void Close();

 private:
  void CheckConnection();

  std::string url_;
  std::string user_;
  std::string password_;
  // 使用的数据库名
  std::string schema_;
  std::size_t size_;
  std::queue<std::unique_ptr<SqlConnection>> connections_;
  std::mutex mtx_;
  std::condition_variable cond_;
  std::atomic<bool> stop_;
  std::thread heartbeat_;
};

struct UserInfo {
  std::string name;
  std::string pwd;
  int uid;
  std::string email;
};

class MysqlDao {
 public:
  MysqlDao();
  ~MysqlDao();
  int RegisterUser(const std::string& name, const std::string& email,
                   const std::string& pwd);
  bool CheckEmail(const std::string& name, const std::string email);
  bool UpdatePwd(const std::string& name, const std::string new_pwd);
  bool CheckPwd(const std::string& email, const std::string pwd,
                UserInfo& user_info);

 private:
  std::unique_ptr<MysqlPool> pool_;
};