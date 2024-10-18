#pragma once

#include "MysqlDao.hpp"
#include "Singleton.hpp"
#include "utilities.hpp"

class MysqlManager : public Singleton<MysqlManager> {
  friend class Singleton<MysqlManager>;

 public:
  ~MysqlManager();

  int RegisterUser(const std::string& name, const std::string& email,
                   const std::string& pwd);
  bool CheckEmail(const std::string& name, const std::string& email);
  bool UpdatePwd(const std::string& name, const std::string& pwd);

  bool CheckPwd(const std::string& email, const std::string& pwd,
                UserInfo& userInfo);
  // bool TestProcedure(const std::string& email, int& uid, std::string& name);

 private:
  MysqlManager();
  MysqlDao dao_;
};