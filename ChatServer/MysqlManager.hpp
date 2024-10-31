#pragma once

#include "MysqlDao.hpp"
#include "Singleton.hpp"
#include "utilities.hpp"

class UserInfo;
class ApplyInfo;

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
  std::shared_ptr<UserInfo> GetUser(int uid);
  std::shared_ptr<UserInfo> GetUser(std::string name);
  bool GetApplyList(int touid,
                    std::vector<std::shared_ptr<ApplyInfo>>& applyList,
                    int begin, int limit = 10);
  bool GetFriendList(int self_id,
                     std::vector<std::shared_ptr<UserInfo>>& user_info_list);
  bool AddFriendApply(const int& from, const int& to);
  bool AuthFriendApply(const int& from, const int& to);
  bool AddFriend(const int& from, const int& to, std::string back_name);

 private:
  MysqlManager();
  MysqlDao dao_;
};