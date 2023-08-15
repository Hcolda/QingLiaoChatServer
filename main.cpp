#include <iostream>
#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include "network.hpp"
#include "socketFunctions.h"
#include "init.h"

Log::Logger serverLogger;
qls::Network serverNetwork;
qls::SocketFunction serverSocketFunction;
qcrypto::pkey::PrivateKey serverPrivateKey(qcrypto::pkey::KeyGenerator::generateRSA(2048));

/*
* 待定清单：
* 1. socketFunction需要修改（加入数据包），加入加密功能
* 2. config 需要做
* 3. 加密功能完成之后登录注册的搭建
*/

int main()
{
    return qls::init();
}