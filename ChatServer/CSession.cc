#include "CSession.hpp"

#include "CServer.hpp"
#include "LogicSystem.hpp"
#include "MsgNode.hpp"

CSession::CSession(net::io_context& ioc, CServer* server)
    : socket_(ioc),
      server_(server),
      close_(false),
      head_parsed_(false),
      user_uid_(0) {
  boost::uuids::uuid id = boost::uuids::random_generator()();
  session_id_ = boost::uuids::to_string(id);
}

CSession::~CSession() {}

tcp::socket& CSession::GetSocket() { return socket_; }

std::string CSession::GetSeesionId() const { return session_id_; }

void CSession::SetUserId(int id) { user_uid_ = id; }

int CSession::GetUserId() const { return user_uid_; }

void CSession::Start() { AsyncReadHead(kHeadTotalLen); }

void CSession::Send(char* msg, short max_length, short msgid) {
  std::lock_guard<std::mutex> lock(send_lock_);
  int send_que_size = send_que_.size();
  if (send_que_size > kMaxSendQue) {
    std::cout << "session: " << session_id_ << " send que fulled, size is "
              << kMaxSendQue << std::endl;
    return;
  }

  send_que_.push(std::make_shared<SendNode>(msg, max_length, msgid));
  if (send_que_size > 0) {
    return;
  }
  auto& msgnode = send_que_.front();
  boost::asio::async_write(
      socket_, boost::asio::buffer(msgnode->data_, msgnode->total_len_),
      [this](boost::system::error_code ec, size_t) {
        this->HandleWrite(ec, shared_from_this());
      });
}

void CSession::Send(std::string msg, short msgid) {
  std::lock_guard<std::mutex> lock(send_lock_);
  int send_que_size = send_que_.size();
  if (send_que_size > kMaxSendQue) {
    std::cout << "session: " << session_id_ << " send que fulled, size is "
              << kMaxSendQue << std::endl;
    return;
  }

  send_que_.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
  if (send_que_size > 0) {
    return;
  }
  auto& msgnode = send_que_.front();
  boost::asio::async_write(
      socket_, boost::asio::buffer(msgnode->data_, msgnode->total_len_),
      [this](boost::system::error_code ec, size_t) {
        this->HandleWrite(ec, shared_from_this());
      });
}

void CSession::Close() {
  socket_.close();
  close_ = true;
}

void CSession::AsyncReadBody(int total_len) {
  auto self = shared_from_this();
  AsyncReadFull(total_len, [self, this, total_len](boost::system::error_code ec,
                                                   size_t bytes_transfered) {
    try {
      if (ec) {
        std::cout << "handle read failed, error is " << ec.what() << std::endl;
        Close();
        server_->ClearSession(session_id_);
        return;
      }
      if (bytes_transfered < total_len) {
        std::cout << "read length not match, read [" << bytes_transfered
                  << "] , total [" << total_len << "]" << std::endl;
        Close();
        server_->ClearSession(session_id_);
        return;
      }
      memcpy(recv_msg_node_->data_, data_, bytes_transfered);
      recv_msg_node_->curr_len_ += bytes_transfered;
      recv_msg_node_->data_[recv_msg_node_->total_len_] = '\0';
      LogicSystem::GetInstance()->PostMsgToQue(
          std::make_shared<LogicNode>(self, recv_msg_node_));
      AsyncReadHead(kHeadTotalLen);

    } catch (std::exception& e) {
      std::cout << "Exception code is " << e.what() << std::endl;
    }
  });
}

void CSession::AsyncReadHead(int total_len) {
  auto self = shared_from_this();
  AsyncReadFull(kHeadTotalLen, [self, this](boost::system::error_code ec,
                                            size_t bytes_transfered) {
    try {
      if (ec) {
        std::cout << "Handle read failed, error is " << ec.what() << std::endl;
        Close();
        server_->ClearSession(session_id_);
        return;
      }

      if (bytes_transfered < kHeadTotalLen) {
        std::cout << "read length not match, read [" << bytes_transfered
                  << "] , total [" << kHeadTotalLen << "]" << std::endl;
        Close();
        server_->ClearSession(session_id_);
        return;
      }

      recv_head_node_->Clear();
      memcpy(recv_head_node_->data_, data_, bytes_transfered);

      // 获取头部MSGID数据
      short msg_id = 0;
      memcpy(&msg_id, recv_head_node_->data_, kHeadIdLen);
      // 网络字节序转化为本地字节序
      msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
      std::cout << "msg_id is " << msg_id << std::endl;
      // id非法
      if (msg_id > kMaxLength) {
        std::cout << "Invalid msg_id is " << msg_id << std::endl;
        server_->ClearSession(session_id_);
        return;
      }
      short msg_len = 0;
      memcpy(&msg_len, recv_head_node_->data_ + kHeadIdLen, kHeadDataLen);
      // 网络字节序转化为本地字节序
      msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
      std::cout << "msg_len is " << msg_len << std::endl;

      // 消息长度非法
      if (msg_len > kMaxLength) {
        std::cout << "Invalid data length is " << msg_len << std::endl;
        server_->ClearSession(session_id_);
        return;
      }

      recv_msg_node_ = std::make_shared<RecvNode>(msg_len, msg_id);
      AsyncReadBody(msg_len);
    } catch (std::exception& e) {
      std::cout << "Exception: " << e.what() << std::endl;
    }
  });
}

// 读取完整长度
void CSession::AsyncReadFull(
    std::size_t maxLength,
    std::function<void(const boost::system::error_code&, std::size_t)>
        handler) {
  ::memset(data_, 0, kMaxLength);
  AsyncReadLen(0, maxLength, handler);
}

// 读取指定字节
void CSession::AsyncReadLen(
    std::size_t read_len, std::size_t total_len,
    std::function<void(const boost::system::error_code&, std::size_t)>
        handler) {
  auto self = shared_from_this();
  socket_.async_read_some(
      net::buffer(data_ + read_len, total_len - read_len),
      [=](boost::system::error_code ec, size_t bytes_transfered) {
        if (ec) {
          handler(ec, read_len + bytes_transfered);
          return;
        }
        if (read_len + bytes_transfered >= total_len) {
          handler(ec, read_len + bytes_transfered);
          return;
        }
        AsyncReadLen(read_len + bytes_transfered, total_len, handler);
      });
}

void CSession::HandleWrite(const boost::system::error_code& ec,
                           std::shared_ptr<CSession> shared_self) {
  try {
    if (!ec) {
      std::lock_guard<std::mutex> lock(send_lock_);
      send_que_.pop();
      if (!send_que_.empty()) {
        auto& msgnode = send_que_.front();
        net::async_write(socket_,
                         net::buffer(msgnode->data_, msgnode->total_len_),
                         [this](boost::system::error_code ec, size_t) {
                           this->HandleWrite(ec, shared_from_this());
                         });
      }
    } else {
      std::cout << "handle write failed, error is " << ec.what() << std::endl;
      Close();
      server_->ClearSession(session_id_);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}