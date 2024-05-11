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
find_package(kcp CONFIG REQUIRED)

# include_directories(QingLiaoChatServer
#     include/
#     src/initialization/
#     src/jsonMessageProcess/
#     src/manager/
#     src/network/
#     src/parser/
#     src/room/groupRoom/
#     src/room/privateRoom/
#     src/room/
#     src/socketFunctions/
#     src/user/)

add_library(libmariadbcpp STATIC IMPORTED)
set_target_properties(libmariadbcpp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/mariadb-connector-cpp/build/Release/mariadbcpp-static.lib)

add_executable(QingLiaoChatServer
    "src/main/main.cpp"
    "src/initialization/init.cpp"
    "src/jsonMessageProcess/JsonMsgProcess.cpp"
    "src/manager/manager.cpp"
    "src/network/network.cpp"
    "src/network/dataPackage.cpp"
    "src/network/package.cpp"
    "src/parser/ini.cpp src/parser/Json.cpp"
    "src/room/room.cpp"
    "src/room/groupRoom/groupRoom.cpp"
    "src/room/groupRoom/groupRoomVerification.cpp"
    "src/room/privateRoom/privateRoom.cpp"
    "src/room/privateRoom/friendRoomVerification.cpp"
    "src/socketFunctions/socketFunctions.cpp"
    "src/user/user.cpp"
    "src/room/groupRoom/groupPermission.cpp"
    "src/network/socket.cpp")

set_property(TARGET QingLiaoChatServer PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(QingLiaoChatServer PUBLIC include/)
target_include_directories(QingLiaoChatServer PUBLIC src/initialization/)
target_include_directories(QingLiaoChatServer PUBLIC src/jsonMessageProcess/)
target_include_directories(QingLiaoChatServer PUBLIC src/manager/)
target_include_directories(QingLiaoChatServer PUBLIC src/network/)
target_include_directories(QingLiaoChatServer PUBLIC src/parser/)
target_include_directories(QingLiaoChatServer PUBLIC src/room/groupRoom/)
target_include_directories(QingLiaoChatServer PUBLIC src/room/privateRoom/)
target_include_directories(QingLiaoChatServer PUBLIC src/room/)
target_include_directories(QingLiaoChatServer PUBLIC src/socketFunctions/)
target_include_directories(QingLiaoChatServer PUBLIC src/user/)

target_include_directories(QingLiaoChatServer PUBLIC mariadb-connector-cpp/include/)

target_link_libraries(QingLiaoChatServer PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(QingLiaoChatServer PRIVATE libmariadbcpp)
target_link_libraries(QingLiaoChatServer PRIVATE asio::asio)
target_link_libraries(QingLiaoChatServer PRIVATE kcp::kcp)
