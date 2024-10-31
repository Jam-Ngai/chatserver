#include "UserManager.hpp"

UserManager::~UserManager() { uid_to_session_.clear(); }

std::shared_ptr<CSession> UserManager::GetSession(int uid) {
  std::lock_guard<std::mutex> lock(mtx_);
  auto it = uid_to_session_.find(uid);
  if (it == uid_to_session_.end()) {
    return nullptr;
  }
  return it->second;
}

void UserManager::SetUserSession(int uid, std::shared_ptr<CSession> session) {
  std::lock_guard<std::mutex> lock(mtx_);
  uid_to_session_[uid] = session;
}

void UserManager::RemoveUserSession(int uid) {
  auto uid_str = std::to_string(uid);
  std::lock_guard<std::mutex> lock(mtx_);
  uid_to_session_.erase(uid);
}