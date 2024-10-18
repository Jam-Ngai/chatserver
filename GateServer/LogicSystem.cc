#include "LogicSystem.hpp"

#include "HttpConnection.hpp"
#include "MysqlManager.hpp"
#include "RedisManager.hpp"
#include "VerifyGrpcClient.hpp"
#include "StatusGrpcClient.hpp"

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
      "/get_verifycode", [](std::shared_ptr<HttpConnection> connection) {
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

        if (!src_root.isMember("email")) {
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

  RegisterPost(
      "/user_register", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str =
            boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "Receive body is " << body_str << std::endl;
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
          std::cout << "Failed to parse JSON data!" << std::endl;
          root["error"] = ErrorCodes::Error_Json;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        std::string name = src_root["name"].asString();
        std::string email = src_root["email"].asString();
        std::string pwd = src_root["passwd"].asString();
        std::string confirm = src_root["confirm"].asString();

        // 判断密码和确认密码是否一致
        if (pwd != confirm) {
          std::cout << "Passwords are inconsistent!" << std::endl;
          root["error"] = ErrorCodes::PasswdErr;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        // 先查找redis中email对应的验证码是否合理
        std::string verify_code;
        bool get_success = RedisManager::GetInstance()->Get(
            kCodePrefix + src_root["email"].asString(), verify_code);
        if (!get_success) {
          std::cout << "Verify code expired!" << std::endl;
          root["error"] = ErrorCodes::VerifyExpired;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        if (verify_code != src_root["verifycode"].asString()) {
          std::cout << "Verify code error!" << std::endl;
          root["error"] = ErrorCodes::VerifyCodeErr;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        // 查找数据库判断用户是否存在
        int uid = MysqlManager::GetInstance()->RegisterUser(name, email, pwd);
        if (0 == uid || -1 == uid) {
          std::cout << "User or email exist!" << std::endl;
          root["error"] = ErrorCodes::UserExist;
          std::string jsonstr = root.toStyledString();
          beast::ostream(connection->response_.body()) << jsonstr;
          return;
        }

        root["error"] = 0;
        root["email"] = email;
        root["uid"] = uid;
        root["user"] = name;
        root["passwd"] = pwd;
        root["confirm"] = pwd;
        root["verifycode"] = verify_code;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
      });

  RegisterPost("/reset_pwd", [](std::shared_ptr<HttpConnection> connection) {
    auto body_str =
        beast::buffers_to_string(connection->request_.body().data());
    std::cout << "receive body is " << body_str << std::endl;
    connection->response_.set(http::field::content_type, "text/json");
    Json::Value root;
    Json::Reader reader;
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
      std::cout << "Failed to parse JSON data!" << std::endl;
      root["error"] = ErrorCodes::Error_Json;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return;
    }

    std::string email = src_root["email"].asString();
    std::string name = src_root["name"].asString();
    std::string pwd = src_root["passwd"].asString();

    // 获取redis中的验证码
    std::string verify_code;
    bool get_success =
        RedisManager::GetInstance()->Get(kCodePrefix + email, verify_code);
    if (!get_success) {
      std::cout << " Verify code expired" << std::endl;
      root["error"] = ErrorCodes::VerifyExpired;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return;
    }

    // 对比验证码是否一致
    if (verify_code != src_root["verify_code"].asString()) {
      std::cout << "Verrify code error" << std::endl;
      root["error"] = ErrorCodes::VerifyCodeErr;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return;
    }

    // 查询数据库判断用户名和邮箱是否匹配
    bool email_valid = MysqlManager::GetInstance()->CheckEmail(name, email);
    if (!email_valid) {
      std::cout << "Email invalid" << std::endl;
      root["error"] = ErrorCodes::EmailNotMatch;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return;
    }

    // 更新密码
    bool update_success = MysqlManager::GetInstance()->UpdatePwd(name, pwd);
    if (!update_success) {
      std::cout << " update pwd failed" << std::endl;
      root["error"] = ErrorCodes::PasswdUpFailed;
      std::string jsonstr = root.toStyledString();
      beast::ostream(connection->response_.body()) << jsonstr;
      return;
    }
    std::cout << "Succeed to update password" << pwd << std::endl;
    root["error"] = 0;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = pwd;
    root["verifycode"] = src_root["verifycode"].asString();
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
  });

  //用户登录逻辑
RegisterPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
    auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
    std::cout << "receive body is " << body_str << std::endl;
    connection->response_.set(http::field::content_type, "text/json");
    Json::Value root;
    Json::Reader reader;
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
        std::cout << "Failed to parse JSON data!" << std::endl;
        root["error"] = ErrorCodes::Error_Json;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }

    auto email = src_root["email"].asString();
    auto pwd = src_root["passwd"].asString();
    UserInfo userInfo;
    //查询数据库判断用户名和密码是否匹配
    bool pwd_valid = MysqlManager::GetInstance()->CheckPwd(email, pwd, userInfo);
    if (!pwd_valid) {
        std::cout << "User pwd not match" << std::endl;
        root["error"] = ErrorCodes::PasswdInvalid;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }

    //查询StatusServer找到合适的连接
    auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
    if (reply.error()) {
        std::cout << "Grpc get chat server failed, error is " << reply.error()<< std::endl;
        root["error"] = ErrorCodes::RPCFailed;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }

    std::cout << "Succeed to load userinfo uid is " << userInfo.uid << std::endl;
    root["error"] = 0;
    root["email"] = email;
    root["uid"] = userInfo.uid;
    root["token"] = reply.token();
    root["host"] = reply.host();
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return true;
    });
}
