# QingLiao Chat Server
这是一个氢聊聊天软件项目

## 前置

### 最基本的要求
1. [git](https://git-scm.com/)
2. llvm或者gcc或者msvc等c++编译器
3. [cmake](https://cmake.org/)
4. [vcpkg](https://github.com/microsoft/vcpkg)

### 配置cmake
1. 设置全局变量`VCPKG_ROOT=你的vcpkg路径`
2. 设置全局变量`CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake`  
   （以上是Windows的方法，Linux下需要稍作修改）

### 构建 QingLiao Chat Server
```cmd
cmake -S . -B build
cmake --build build --config Release
```

## 开发规则
1. 必须使用驼峰法命名
2. 必须保证线程安全，在多线程环境下必须使用线程锁 (std::mutex, std::shared_mutex)
3. 必须保证内存安全，尽量使用std::shared_ptr等代理
4. 必须保证无死锁情况发生
5. 未经过测试或者不成熟的技术请勿在项目中直接使用

## TODO
- [x] Network
- [x] Manager
- [x] User
- [ ] Room (Private Room and Group Room)
- [ ] Voice Chat
- [ ] File Transport
- [ ] Permission
- [ ] Chat Bot Library

## 辅助文档
- [FormatForDataPackage.md](./doc/FormatForDataPackage.md)
- [Website.md](./doc/Website.md)

