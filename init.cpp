#include "init.h"
#include <iostream>
#include <Logger.hpp>
#include <asio.hpp>

#include "network.hpp"

Log::Logger logger;
qls::Network network;

int qls::init()
{
    std::system("chcp 65001");
    logger.info("服务器Log系统启动成功！");

	try
	{
		logger.info("服务器监听正在启动！");
		network.run("0.0.0.0", 55555);
	}
	catch (const std::exception& e)
	{
		logger.error(e.what());
	}

    return 0;
}
