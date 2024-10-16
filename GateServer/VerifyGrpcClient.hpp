#pragma once
#include "Singleton.hpp"
#include "message.grpc.pb.h"
#include "utilities.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using message::VerifyRequst;
using message::VerifyResponse;
using message::VerifyService;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient> {
  friend class Singleton<VerifyGrpcClient>;

 public:
  VerifyResponse GetVerifyCode(std::string email);

 private:
  VerifyGrpcClient() {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        "0.0.0.0:50051", grpc::InsecureChannelCredentials());
    stub_ = VerifyService::NewStub(channel);
  }
  std::unique_ptr<VerifyService::Stub> stub_;
};