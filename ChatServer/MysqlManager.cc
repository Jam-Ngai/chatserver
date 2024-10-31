#include "MysqlManager.hpp"

MysqlManager::MysqlManager() {}

MysqlManager::~MysqlManager() {}

int MysqlManager::RegisterUser(const std::string& name,
                               const std::string& email,
                               const std::string& pwd) {
  return dao_.RegisterUser(name, email, pwd);
}

bool MysqlManager::CheckEmail(const std::string& name,
                              const std::string& email) {
  return dao_.CheckEmail(name, email);
}

bool MysqlManager::UpdatePwd(const std::string& name, const std::string& pwd) {
  return dao_.UpdatePwd(name, pwd);
}

bool MysqlManager::CheckPwd(const std::string& email, const std::string& pwd,
                            UserInfo& userInfo) {
  return dao_.CheckPwd(email, pwd, userInfo);
}

std::shared_ptr<UserInfo> MysqlManager::GetUser(int uid) {
  return dao_.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlManager::GetUser(std::string name) {
  return dao_.GetUser(name);
}

bool MysqlManager::GetApplyList(
    int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin,
    int limit) {
  return dao_.GetApplyList(touid, applyList, begin, limit);
}

bool MysqlManager::GetFriendList(
    int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info_list) {
  return dao_.GetFriendList(self_id, user_info_list);
}

bool MysqlManager::AddFriendApply(const int& from, const int& to) {
  return dao_.AddFriendApply(from, to);
}

bool MysqlManager::AuthFriendApply(const int& from, const int& to) {
  return dao_.AuthFriendApply(from, to);
}

bool MysqlManager::AddFriend(const int& from, const int& to,
                             std::string back_name) {
  return dao_.AddFriend(from, to, back_name);
}