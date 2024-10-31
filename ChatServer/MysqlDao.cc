#include "MysqlDao.hpp"

#include "ConfigManager.hpp"
#include "data.hpp"

MysqlPool::MysqlPool(const std::string& url, const std::string& user,
                     const std::string& pwd, const std::string& schema,
                     int size)
    : url_(url),
      user_(user),
      password_(pwd),
      schema_(schema),
      size_(size),
      stop_(false) {
  try {
    for (std::size_t i = 0; i < size; ++i) {
      sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
      sql::Connection* connection = driver->connect(url_, user_, password_);
      connection->setSchema(schema_);
      // 获取当前时间戳
      auto curr_time = std::chrono::system_clock::now().time_since_epoch();
      // 将时间戳转换成秒
      long long timestamp =
          std::chrono::duration_cast<std::chrono::seconds>(curr_time).count();
      connections_.push(std::make_unique<SqlConnection>(connection, timestamp));
    }

    heartbeat_ = std::thread([this]() {
      while (!stop_) {
        CheckConnection();
        std::this_thread::sleep_for(std::chrono::seconds(60));
      }
    });

    std::cout << "Database " << schema << " connected!" << std::endl;
  } catch (sql::SQLException& e) {
    std::cout << "mysql pool init failed" << std::endl;
  }
}

MysqlPool::MysqlPool::~MysqlPool() {
  std::lock_guard<std::mutex> lock(mtx_);
  Close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

void MysqlPool::CheckConnection() {
  std::lock_guard<std::mutex> lock(mtx_);
  std::size_t size = connections_.size();
  // 获取当前时间戳
  auto curr_time = std::chrono::system_clock::now().time_since_epoch();
  // 将时间戳转换成秒
  long long timestamp =
      std::chrono::duration_cast<std::chrono::seconds>(curr_time).count();
  for (std::size_t i; i < size; ++i) {
    auto conn = std::move(connections_.front());
    connections_.pop();

    Defer defer([this, &conn]() { connections_.push(std::move(conn)); });

    if (timestamp - conn->last_time_ < 5) {
      continue;
    }

    try {
      std::unique_ptr<sql::Statement> statement(conn->conn_->createStatement());
      statement->executeQuery("SELECT 1");
      conn->last_time_ = timestamp;
    } catch (sql::SQLException& e) {
      std::cout << "Error keeping connection alive: " << e.what() << std::endl;
      // 重新创建连接并替换旧的连接
      sql::mysql::MySQL_Driver* driver =
          sql::mysql::get_mysql_driver_instance();
      auto* new_conn = driver->connect(url_, user_, password_);
      new_conn->setSchema(schema_);
      conn->conn_.reset(new_conn);
      conn->last_time_ = timestamp;
    }
  }
}

std::unique_ptr<SqlConnection> MysqlPool::GetConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cond_.wait(lock, [this]() {
    if (stop_) {
      return true;
    }
    return !connections_.empty();
  });
  if (stop_) {
    return nullptr;
  }
  std::unique_ptr<SqlConnection> conn(std::move(connections_.front()));
  connections_.pop();
  return conn;
}

void MysqlPool::ReturnConnection(std::unique_ptr<SqlConnection> conn) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (stop_) {
    return;
  }
  connections_.push(std::move(conn));
  cond_.notify_one();
}

void MysqlPool::Close() {
  stop_ = true;
  cond_.notify_all();
}

MysqlDao::MysqlDao() {
  auto& config_manager = ConfigManager::GetInstance();
  std::string host = config_manager["Mysql"]["Host"];
  std::string port = config_manager["Mysql"]["Port"];
  std::string pwd = config_manager["Mysql"]["Passwd"];
  std::string schema = config_manager["Mysql"]["Schema"];
  std::string user = config_manager["Mysql"]["User"];
  pool_.reset(new MysqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() { pool_->Close(); }

int MysqlDao::RegisterUser(const std::string& name, const std::string& email,
                           const std::string& pwd) {
  auto conn = pool_->GetConnection();
  try {
    if (nullptr == conn) {
      return -1;
    }
    // 准备调用存储过程
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->conn_->prepareStatement("CALL reg_user(?,?,?,@result)"));
    // 设置输入参数
    stmt->setString(1, name);
    stmt->setString(2, email);
    stmt->setString(3, pwd);

    // prepareStatement不支持直接注册输出参数,需要使用会话变量或其他方法来获取输出参数的值
    // 执行存储过程
    stmt->execute();
    // 如果存储过程设置了会话变量或者有其他方式获取输出参数的值
    // 可以通过执行select来获取
    // 例如,存储过程设置了一个会话变量@result来存储输出结果,可以这样获取:
    std::unique_ptr<sql::Statement> stmt_result(conn->conn_->createStatement());
    std::unique_ptr<sql::ResultSet> result(
        stmt_result->executeQuery("SELECT @result AS result"));
    if (result->next()) {
      int uid = result->getInt("result");
      std::cout << "Result uid = " << uid << std::endl;
      pool_->ReturnConnection(std::move(conn));
      return uid;
    }
    pool_->ReturnConnection(std::move(conn));
    return -1;
  } catch (sql::SQLException& e) {
    pool_->ReturnConnection(std::move(conn));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return -1;
  }
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string email) {
  auto conn = pool_->GetConnection();
  if (nullptr == conn) {
    return false;
  }
  try {
    std::unique_ptr<sql::PreparedStatement> pre_stmt(
        conn->conn_->prepareStatement("SELECT email FROM user WHERE name =?"));
    pre_stmt->setString(1, name);

    std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
    while (res->next()) {
      std::cout << "Check Email: " << res->getString("email") << std::endl;
      if (email != res->getString("email")) {
        pool_->ReturnConnection(std::move(conn));
        return false;
      }
      pool_->ReturnConnection(std::move(conn));
      return true;
    }
    return false;
  } catch (sql::SQLException& e) {
    pool_->ReturnConnection(std::move(conn));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string new_pwd) {
  auto conn = pool_->GetConnection();
  if (nullptr == conn) {
    return false;
  }
  try {
    std::unique_ptr<sql::PreparedStatement> pre_stmt(
        conn->conn_->prepareStatement(
            "UPDATE user SET pwd = ? WHERE name = ?"));
    pre_stmt->setString(1, new_pwd);
    pre_stmt->setString(2, name);
    int update_cnt = pre_stmt->executeUpdate();
    std::cout << "Updated rows: " << update_cnt << std::endl;
    pool_->ReturnConnection(std::move(conn));
    return true;
  } catch (sql::SQLException& e) {
    pool_->ReturnConnection(std::move(conn));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::CheckPwd(const std::string& email, const std::string pwd,
                        UserInfo& user_info) {
  auto conn = pool_->GetConnection();
  if (nullptr == conn) {
    return false;
  }
  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });
  try {
    std::unique_ptr<sql::PreparedStatement> pre_stmt(
        conn->conn_->prepareStatement("SELECT * FROM user WHERE email=?"));
    pre_stmt->setString(1, email);
    std::unique_ptr<sql::ResultSet> res(pre_stmt->executeQuery());
    if (res->next()) {
      if (pwd != res->getString("pwd")) {
        return false;
      }
    }
    user_info.email = res->getString("email");
    user_info.name = res->getString("name");
    user_info.pwd = res->getString("pwd");
    user_info.uid = res->getInt("uid");
    return true;

  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid) {
  auto conn = pool_->GetConnection();
  if (conn = nullptr) return nullptr;

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->conn_->prepareStatement("SELECT * FROM user WHERE uid=?"));
    pstmt->setInt(1, uid);
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    std::shared_ptr<UserInfo> user_ptr = nullptr;
    while (res->next()) {
      user_ptr.reset(new UserInfo);
      user_ptr->pwd = res->getString("pwd");
      user_ptr->email = res->getString("email");
      user_ptr->name = res->getString("name");
      user_ptr->nick = res->getString("nick");
      user_ptr->desc = res->getString("desc");
      user_ptr->sex = res->getInt("sex");
      user_ptr->icon = res->getString("icon");
      user_ptr->uid = uid;
      break;
    }
    return user_ptr;
  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return nullptr;
  }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(std::string name) {
  auto conn = pool_->GetConnection();
  if (conn = nullptr) return nullptr;

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->conn_->prepareStatement("SELECT * FROM user WHERE name=?"));
    pstmt->setString(1, name);
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    std::shared_ptr<UserInfo> user_ptr = nullptr;
    while (res->next()) {
      user_ptr.reset(new UserInfo);
      user_ptr->pwd = res->getString("pwd");
      user_ptr->email = res->getString("email");
      user_ptr->name = res->getString("name");
      user_ptr->nick = res->getString("nick");
      user_ptr->desc = res->getString("desc");
      user_ptr->sex = res->getInt("sex");
      user_ptr->icon = res->getString("icon");
      user_ptr->uid = res->getInt("uid");
      break;
    }
    return user_ptr;
  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return nullptr;
  }
}

// 获取申请列表
bool MysqlDao::GetApplyList(int to_uid,
                            std::vector<std::shared_ptr<ApplyInfo>>& applyList,
                            int begin, int limit) {
  auto conn = pool_->GetConnection();
  if (conn == nullptr) {
    return false;
  }

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    // 准备SQL语句, 根据起始id和限制条数返回列表
    std::unique_ptr<sql::PreparedStatement> pstmt(conn->conn_->prepareStatement(
        "select apply.from_uid, apply.status, user.name, "
        "user.nick, user.sex from friend_apply as apply join user on "
        "apply.from_uid = user.uid where apply.to_uid = ? "
        "and apply.id > ? order by apply.id ASC LIMIT ? "));

    pstmt->setInt(1, to_uid);  // 将uid替换为你要查询的uid
    pstmt->setInt(2, begin);   // 起始id
    pstmt->setInt(3, limit);   // 偏移量
    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    // 遍历结果集
    while (res->next()) {
      auto name = res->getString("name");
      auto uid = res->getInt("from_uid");
      auto status = res->getInt("status");
      auto nick = res->getString("nick");
      auto sex = res->getInt("sex");
      auto apply_ptr =
          std::make_shared<ApplyInfo>(uid, name, "", "", nick, sex, status);
      applyList.push_back(apply_ptr);
    }
    return true;
  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

// 获取好友列表
bool MysqlDao::GetFriendList(
    int self_id, std::vector<std::shared_ptr<UserInfo>>& user_info_list) {
  auto conn = pool_->GetConnection();
  if (conn == nullptr) {
    return false;
  }

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    // 准备SQL语句, 根据起始id和限制条数返回列表
    std::unique_ptr<sql::PreparedStatement> pstmt(conn->conn_->prepareStatement(
        "select * from friend where self_id = ? "));

    pstmt->setInt(1, self_id);  // 将uid替换为你要查询的uid

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    // 遍历结果集
    while (res->next()) {
      auto friend_id = res->getInt("friend_id");
      auto back = res->getString("back");
      // 再一次查询friend_id对应的信息
      auto user_info = GetUser(friend_id);
      if (user_info == nullptr) {
        continue;
      }

      user_info->back = user_info->name;
      user_info_list.push_back(user_info);
    }
    return true;
  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }

  return true;
}

bool MysqlDao::AddFriendApply(const int& from, const int& to) {
  auto conn = pool_->GetConnection();
  if (conn == nullptr) {
    return false;
  }

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    // 准备SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(conn->conn_->prepareStatement(
        "INSERT INTO friend_apply (from_uid, to_uid) values (?,?) "
        "ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid"));
    pstmt->setInt(1, from);  // from id
    pstmt->setInt(2, to);
    // 执行更新
    int rowAffected = pstmt->executeUpdate();
    if (rowAffected < 0) {
      return false;
    }
    return true;
  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }

  return true;
}

bool MysqlDao::AuthFriendApply(const int& from, const int& to) {
  auto conn = pool_->GetConnection();
  if (conn == nullptr) {
    return false;
  }

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    // 准备SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        conn->conn_->prepareStatement("UPDATE friend_apply SET status = 1 "
                                      "WHERE from_uid = ? AND to_uid = ?"));
    // 反过来的申请时from，验证时to
    pstmt->setInt(1, to);  // from id
    pstmt->setInt(2, from);
    // 执行更新
    int rowAffected = pstmt->executeUpdate();
    if (rowAffected < 0) {
      return false;
    }
    return true;
  } catch (sql::SQLException& e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }

  return true;
}

bool MysqlDao::AddFriend(const int& from, const int& to,
                         std::string back_name) {
  auto conn = pool_->GetConnection();
  if (conn == nullptr) {
    return false;
  }

  Defer defer([this, &conn]() { pool_->ReturnConnection(std::move(conn)); });

  try {
    // 开始事务
    conn->conn_->setAutoCommit(false);

    // 准备第一个SQL语句, 插入认证方好友数据
    std::unique_ptr<sql::PreparedStatement> pstmt(conn->conn_->prepareStatement(
        "INSERT IGNORE INTO friend(self_id, friend_id, back) "
        "VALUES (?, ?, ?) "));
    // 反过来的申请时from，验证时to
    pstmt->setInt(1, from);  // from id
    pstmt->setInt(2, to);
    pstmt->setString(3, back_name);
    // 执行更新
    int rowAffected = pstmt->executeUpdate();
    if (rowAffected < 0) {
      conn->conn_->rollback();
      return false;
    }

    // 准备第二个SQL语句，插入申请方好友数据
    std::unique_ptr<sql::PreparedStatement> pstmt2(
        conn->conn_->prepareStatement(
            "INSERT IGNORE INTO friend(self_id, friend_id, back) "
            "VALUES (?, ?, ?) "));
    // 反过来的申请时from，验证时to
    pstmt2->setInt(1, to);  // from id
    pstmt2->setInt(2, from);
    pstmt2->setString(3, "");
    // 执行更新
    int rowAffected2 = pstmt2->executeUpdate();
    if (rowAffected2 < 0) {
      conn->conn_->rollback();
      return false;
    }

    // 提交事务
    conn->conn_->commit();
    std::cout << "addfriend insert friends success" << std::endl;

    return true;
  } catch (sql::SQLException& e) {
    // 如果发生错误，回滚事务
    if (conn) {
      conn->conn_->rollback();
    }
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }

  return true;
}