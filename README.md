

### **项目描述**

这是一个全栈的即时通讯项目，前端基于QT实现气泡聊天对话框，通过`QListWidget`实现好友列表，利用`GridLayout`和`QPainter`封装气泡聊天框组件，基于`QT network`模块封装`http`和`tcp`服务。支持添加好友，好友通信，聊天记录展示等功能，仿微信布局并使用`qss`优化界面。

后端采用分布式设计，分为`GateServer`网关服务，多个`ChatServer`聊天服务，`StatusServer`状态服务以及`VerifyServer`验证服务。

各服务通过`grpc`通信，支持断线重连。`GateServer`网关对外采用`http`服务，负责处理用户登录和注册功能。登录时`GateServer`从`StatusServer`查询聊天服务达到负载均衡，`ChatServer`聊天服务采用`asio`实现tcp可靠长链接异步通信和转发, 采用多线程模式封装`iocontext`池提升并发性能。数据存储采用mysql服务，并基于`mysqlconnector`库封装连接池，同时封装`redis`连接池处理缓存数据，以及`grpc`连接池保证多服务并发访问。

经测试单服务器支持8000连接，多服务器分布部署可支持1W~2W活跃用户。
