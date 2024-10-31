#pragma once
#include "utilities.hpp"

class CServer;
class LogicSystem;
class MsgNode;
class RecvNode;
class SendNode;

class CSession : public std::enable_shared_from_this<CSession> {
 public:
  CSession(net::io_context& ioc, CServer* server);
  ~CSession();

  tcp::socket& GetSocket();
  std::string GetSeesionId() const;
  void SetUserId(int id);
  int GetUserId() const;
  void Start();
  void Send(char* msg, short max_length, short msgid);
  void Send(std::string msg, short msgid);
  void Close();
  void AsyncReadBody(int total_len);
  void AsyncReadHead(int total_len);

 private:
  void AsyncReadFull(
      std::size_t maxLength,
      std::function<void(const boost::system::error_code&, std::size_t)>
          handler);
  void AsyncReadLen(
      std::size_t read_len, std::size_t total_len,
      std::function<void(const boost::system::error_code&, std::size_t)>
          handler);

  void HandleWrite(const boost::system::error_code& error,
                   std::shared_ptr<CSession> shared_self);

  tcp::socket socket_;
  std::string session_id_;
  char data_[kMaxLength];
  CServer* server_;
  bool close_;
  std::queue<std::shared_ptr<SendNode> > send_que_;
  std::mutex send_lock_;
  // 收到的消息结构
  std::shared_ptr<RecvNode> recv_msg_node_;
  bool head_parsed_;
  // 收到的头部结构
  std::shared_ptr<MsgNode> recv_head_node_;
  int user_uid_;
};

class LogicNode {
  friend class LogicSystem;

 public:
  LogicNode(std::shared_ptr<CSession> session,
            std::shared_ptr<RecvNode> recvnode)
      : session_(session), recvnode_(recvnode) {};

 private:
  std::shared_ptr<CSession> session_;
  std::shared_ptr<RecvNode> recvnode_;
};