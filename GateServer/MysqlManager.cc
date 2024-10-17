#include "MysqlManager.hpp"

MysqlManager::MysqlManager() {}

MysqlManager::~MysqlManager() {}

int MysqlManager::RegisterUser(const std::string& name,
                               const std::string& email,
                               const std::string& pwd) {
  return dao_.RegisterUser(name, email, pwd);
}

// bool MysqlManager::CheckEmail(const std::string& name,
//                               const std::string& email) {}
// bool MysqlManager::UpdatePwd(const std::string& name,
//                              const std::string& email) {}
// bool MysqlManager::CheckPwd(const std::string& email, const std::string& pwd,
//                             UserInfo& userInfo) {}
// bool MysqlManager::TestProcedure(const std::string& email, int& uid,
//                                  std::string& name) {}
