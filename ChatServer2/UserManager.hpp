#pragma once

#include "Singleton.hpp"
#include "utilities.hpp"

class CSession;

class UserManager : public Singleton<UserManager> {
  friend class Singleton<UserManager>;

 public:
  ~UserManager();
  std::shared_ptr<CSession> GetSession(int uid);
  void SetUserSession(int uid, std::shared_ptr<CSession> session);
  void RemoveUserSession(int uid);

 private:
  UserManager() {};
  std::mutex mtx_;
  std::unordered_map<int, std::shared_ptr<CSession>> uid_to_session_;
};