#include "VerifyGrpcClient.hpp"

VerifyResponse VerifyGrpcClient::GetVerifyCode(std::string email) {
  ClientContext context;
  VerifyResponse reply;
  VerifyRequst request;
  request.set_email(email);
  Status state = stub_->GetVerifyCode(&context, request, &reply);
  if (state.ok()) {
    return reply;
  } else {
    reply.set_error(ErrorCodes::RPCFailed);
    return reply;
  }
}