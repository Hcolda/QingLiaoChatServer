#include <iostream>
#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <thread>
#include <chrono>

#include "network.h"
#include "socketFunctions.h"
#include "init.h"
#include "Ini.h"

// 服务器log系统
Log::Logger serverLogger;
// 服务器网络系统
qls::Network serverNetwork;
// 服务器socket函数系统（废弃）
qls::SocketFunction serverSocketFunction;
// 服务器rsa密钥（废弃）
qcrypto::pkey::PrivateKey serverPrivateKey(qcrypto::pkey::KeyGenerator::generateRSA(2048));
// ini配置
qini::INIObject serverIni;

int main()
{
    using namespace std::chrono;

    int code = qls::init();
    std::this_thread::sleep_for(1s);
    return code;
}