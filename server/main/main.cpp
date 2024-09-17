#include <iostream>
#include <logger.hpp>
#include <thread>
#include <chrono>

#include "network.h"
#include "socketFunctions.h"
#include "init.h"
#include "Ini.h"
#include "manager.h"
#include "SQLProcess.hpp"

// server log system
Log::Logger serverLogger;
// ini config
qini::INIObject serverIni;
// server network system
qls::Network serverNetwork;
// manager
qls::Manager serverManager;

int main()
{
    using namespace std::chrono;

    int code = qls::init();
    return code;
}