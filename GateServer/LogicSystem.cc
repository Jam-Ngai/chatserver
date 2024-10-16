#include "LogicSystem.hpp"

#include "HttpConnection.hpp"
#include "VerifyGrpcClient.hpp"

bool LogicSystem::HandleGet(std::string url,
                            std::shared_ptr<HttpConnection> connection) {
  if (get_handlers_.find(url) == get_handlers_.end()) {
    return false;
  }
  get_handlers_[url](connection);
  return true;
}

bool LogicSystem::HandlePost(std::string url,
                             std::shared_ptr<HttpConnection> connection) {
  if (post_handlers_.find(url) == post_handlers_.end()) {
    return false;
  }
  post_handlers_[url](connection);
  return true;
}

void LogicSystem::RegisterGet(std::string url, HttpHandler handler) {
  get_handlers_[url] = handler;
}

void LogicSystem::RegisterPost(std::string url, HttpHandler handler) {
  post_handlers_[url] = handler;
}

LogicSystem::LogicSystem() {
  RegisterGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
    beast::ostream(connection->response_.body())
        << "receive get_test register" << std::endl;
    int i = 0;
    for (auto& elem : connection->get_params_) {
      i++;
      beast::ostream(connection->response_.body())
          << "param" << i << " key is " << elem.first;
      beast::ostream(connection->response_.body())
          << ", " << " value is " << elem.second << std::endl;
    }
  });

  RegisterPost(
      "/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
        std::string body_str =
            beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;

        bool success = reader.parse(body_str, src_root);
        if (!success) {
          std::cout << "Failed to parse JSON data!" << std::endl;
          root["error"] = ErrorCodes::Error_Json;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        if (src_root.isMember("email")) {
          std::cout << "Failed to parse JSON data!" << std::endl;
          root["error"] = ErrorCodes::Error_Json;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        std::string email = src_root["email"].asString();
        VerifyResponse response =
            VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
        std::cout << "email is " << email << std::endl;
        root["error"] = response.error();
        root["email"] = src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
      });
}
