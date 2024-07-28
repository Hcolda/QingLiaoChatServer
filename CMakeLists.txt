cmake_minimum_required(VERSION 3.24)

if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(QingLiaoChatServer)

find_package(OpenSSL REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-mariadb-connector-cpp CONFIG REQUIRED)

if(MINGW)
  find_library(WSOCK32_LIBRARY wsock32)
  find_library(WS2_32_LIBRARY ws2_32)
endif()

add_executable(QingLiaoChatServer
    "src/main/main.cpp"
    "src/initialization/init.cpp"
    "src/jsonMessageProcess/JsonMsgProcess.cpp"
    "src/manager/manager.cpp"
    "src/manager/dataManager.cpp"
    "src/manager/verificationManager.cpp"
    "src/network/network.cpp"
    "src/network/dataPackage.cpp"
    "src/network/package.cpp"
    "src/parser/Ini.cpp"
    "src/parser/Json.cpp"
    "src/room/room.cpp"
    "src/room/groupRoom/groupRoom.cpp"
    "src/room/groupRoom/groupRoomVerification.cpp"
    "src/room/privateRoom/privateRoom.cpp"
    "src/room/privateRoom/friendRoomVerification.cpp"
    "src/socketFunctions/socketFunctions.cpp"
    "src/user/user.cpp"
    "src/room/groupRoom/groupPermission.cpp"
    "src/network/socket.cpp"
    "src/input/input.cpp"
    "src/input/inputCommands.cpp"
    "src/error/qls_error.cpp")

set_property(TARGET QingLiaoChatServer PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(QingLiaoChatServer PUBLIC 
  include/
  src/initialization/
  src/jsonMessageProcess/
  src/manager/
  src/network/
  src/parser/
  src/room/groupRoom/
  src/room/privateRoom/
  src/room/
  src/socketFunctions/
  src/user/
  src/input/
  src/error/)

target_link_libraries(QingLiaoChatServer PRIVATE
  OpenSSL::SSL
  OpenSSL::Crypto
  unofficial::mariadb-connector-cpp::mariadbcpp
  asio::asio)

if(MINGW)
  target_link_libraries(QingLiaoChatServer PRIVATE wsock32 ws2_32)
  message(STATUS "Linking ws2_32 - done")
endif()
