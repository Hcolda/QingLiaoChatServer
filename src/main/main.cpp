#include <iostream>
#include <Logger.hpp>
#include <thread>
#include <chrono>

#include "network.h"
#include "socketFunctions.h"
#include "init.h"
#include "Ini.h"
#include "manager.h"
#include "SQLProcess.hpp"

// 服务器log系统
Log::Logger serverLogger;
// ini配置
qini::INIObject serverIni;
// 服务器网络系统
qls::Network serverNetwork;
// manager
qls::Manager serverManager;

int main()
{
    using namespace std::chrono;

    int code = qls::init();
    std::this_thread::sleep_for(1s);
    return code;
}