#include "MsgNode.hpp"

MsgNode::MsgNode(short max_len) : total_len_(max_len), curr_len_(0) {
  data_ = new char[total_len_ + 1]();
  data_[total_len_] = '\0';
}

MsgNode::~MsgNode() { delete[] data_; }

void MsgNode::Clear() {
  ::memset(data_, 0, total_len_);
  curr_len_ = 0;
}

RecvNode::RecvNode(short max_len, short msg_id)
    : MsgNode(max_len), msg_id_(msg_id) {}

SendNode::SendNode(const char* msg, short max_len, short msg_id)
    : MsgNode(max_len + kHeadTotalLen), msg_id_(msg_id) {
  // 先发送id, 转为网络字节序
  short msg_id_host =
      boost::asio::detail::socket_ops::host_to_network_short(msg_id);
  memcpy(data_, &msg_id_host, kHeadIdLen);
  // 转为网络字节序
  short max_len_host =
      boost::asio::detail::socket_ops::host_to_network_short(max_len);
  memcpy(data_ + kHeadIdLen, &max_len_host, kHeadDataLen);
  memcpy(data_ + kHeadTotalLen, msg, max_len);
}