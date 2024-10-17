const config_module = require("./config");
const Redis = require("ioredis");

// 创建Redis客户端实例
const RedisCli = new Redis({
  host: config_module.redis_host, // Redis服务器主机名
  port: config_module.redis_port, // Redis服务器端口号
  password: config_module.redis_passwd, // Redis密码
});
