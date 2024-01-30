# QingLiao Server
这是一个氢聊聊天软件项目

## 前置
1. [openssl](https://github.com/openssl/openssl)
2. [asio](https://github.com/chriskohlhoff/asio)
3. [mariadb](https://github.com/mariadb-corporation/mariadb-connector-cpp)

## 开发规则
1. 必须使用驼峰法命名
2. 必须保证线程安全，在多线程环境下必须使用线程锁 (std::mutex, std::shared_mutex)
3. 必须保证内存安全，尽量使用std::shared_ptr等代理
4. 必须保证无死锁情况发生

## TODO
- [x] Network
- [x] Manager
- [x] User
- [ ] Room (Private Room and Group Room)
- [ ] Voice Chat
- [ ] File Transport
- [ ] Permission
- [ ] Chat Bot Library
