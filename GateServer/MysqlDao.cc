#include "MysqlDao.hpp"

#include "ConfigManager.hpp"

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
    std::unique_ptr<sql::PreparedStatement> statement(
        conn->conn_->prepareStatement("CALL reg_user(?,?,?,@result)"));
    // 设置输入参数
    statement->setString(1, name);
    statement->setString(2, email);
    statement->setString(3, pwd);

    // prepareStatement不支持直接注册输出参数,需要使用会话变量或其他方法来获取输出参数的值
    // 执行存储过程
    statement->execute();
    // 如果存储过程设置了会话变量或者有其他方式获取输出参数的值
    // 可以通过执行select来获取
    // 例如,存储过程设置了一个会话变量@result来存储输出结果,可以这样获取:
    std::unique_ptr<sql::Statement> statemen_result(
        conn->conn_->createStatement());
    std::unique_ptr<sql::ResultSet> result(
        statemen_result->executeQuery("SELECT @result AS result"));
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

// bool MysqlDao::CheckEmail(const std::string& name, const std::string email) {}
// bool MysqlDao::UpdatePwd(const std::string& name, const std::string new_pwd) {}
// bool MysqlDao::CheckPwd(const std::string& name, const std::string new_pwd,
//                         UserInfo& user_info) {}