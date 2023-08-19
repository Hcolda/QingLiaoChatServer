#include "init.h"

#include <functional>

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::SocketFunction serverSocketFunction;
extern qcrypto::pkey::PrivateKey serverPrivateKey;

namespace qls
{
    int init()
    {
        std::system("chcp 65001");
        serverLogger.info("服务器Log系统启动成功！");

        try
        {
            serverLogger.info("服务器监听正在启动，地址：", "0.0.0.0", ":", 55555);
            serverNetwork.setFunctions(std::bind(&SocketFunction::accecptFunction, &serverSocketFunction, std::placeholders::_1),
                                       std::bind(&SocketFunction::receiveFunction, &serverSocketFunction, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                       std::bind(&SocketFunction::closeFunction, &serverSocketFunction, std::placeholders::_1));
            serverNetwork.run("0.0.0.0", 55555);
        }
        catch (const std::exception& e)
        {
            serverLogger.error(e.what());
            return -1;
        }

        return 0;
    }
}
