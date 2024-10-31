#pragma once

#include "Singleton.hpp"
#include "utilities.hpp"

class CSession;
class LogicNode;
class UserInfo;
class ApplyInfo;

using FunCallback =
    std::function<void(std::shared_ptr<CSession>, const uint16_t& msg_id,
                       const std::string& msg_data)>;

class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

 public:
  ~LogicSystem();
  void PostMsgToQue(std::shared_ptr<LogicNode> msg);

 private:
  LogicSystem();
  void DealMsg();
  void RegisterCallback();
  bool GetBaseInfo(const std::string& base_key, int uid,
                   std::shared_ptr<UserInfo>& userinfo);
  void LoginHandler(std::shared_ptr<CSession> session, const uint16_t& msg_id,
                    const std::string& msg_data);
  bool GetFriendApplyInfo(int to_uid,
                          std::vector<std::shared_ptr<ApplyInfo>>& list);
  bool GetFriendList(int self_id,
                     std::vector<std::shared_ptr<UserInfo>>& friend_list);
  void SearchInfo(std::shared_ptr<CSession> session, const short& msg_id,
                  const std::string& msg_data);
  void AddFriendApply(std::shared_ptr<CSession> session, const short& msg_id,
                      const std::string& msg_data);
  void AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id,
                       const std::string& msg_data);
  void DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id,
                       const std::string& msg_data);
  bool IsPureDigit(const std::string& str);

  std::thread worker_;
  std::queue<std::shared_ptr<LogicNode>> msg_que_;
  std::mutex mtx_;
  std::condition_variable cond_;
  std::atomic<bool> stop_;
  std::unordered_map<uint16_t, FunCallback> func_callbacks_;
};