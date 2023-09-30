#include "init.h"

#include <Logger.hpp>
#include <functional>
#include <filesystem>
#include <fstream>

#include "SQLProcess.hpp"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::SocketFunction serverSocketFunction;
extern qcrypto::pkey::PrivateKey serverPrivateKey;
extern qini::INIObject serverIni;

namespace qls
{
    void Init::createConfig()
    {
        std::filesystem::create_directory("./config");

        if (!std::filesystem::exists("./config/config.ini"))
        {
            std::ofstream outfile("./config/config.ini");

            qini::INIObject ini;
            ini["server"]["host"] = "0.0.0.0";
            ini["server"]["port"] = std::to_string(55555);

            ini["mysql"]["host"] = "127.0.0.1";
            ini["mysql"]["port"] = std::to_string(3306);
            ini["mysql"]["username"] = "";
            ini["mysql"]["password"] = "";

            outfile << qini::INIWriter::fastWrite(ini);
        }
        return;
    }

    qini::INIObject Init::readConfig()
    {
        std::ifstream infile("./config/config.ini");
        if (!infile) throw std::runtime_error("can't open file");
        return qini::INIParser::fastParse(infile);
    }

    int init()
    {
        std::system("chcp 65001");
        serverLogger.info("服务器Log系统启动成功！");

        try
        {
            serverLogger.info("正在读取配置文件...");
            serverIni = Init::readConfig();

            if (std::stoll(serverIni["mysql"]["port"]) > 65535)
                throw std::logic_error("ini配置文件 section:mysql, key:port port过大");
            
            serverLogger.info("配置文件读取成功！");
        }
        catch (const std::exception& e)
        {
            serverLogger.error(e.what());
            Init::createConfig();
            serverLogger.error("请修改配置文件");
            return -1;
        }

        std::shared_ptr<quqisql::SQLDBProcess> s = std::make_shared<quqisql::SQLDBProcess>();
        try
        {
            serverLogger.info("正在加载sql...");
            s->setSQLServerInfo(serverIni["mysql"]["username"],
                serverIni["mysql"]["password"],
                "mysql",
                serverIni["mysql"]["host"],
                unsigned short(std::stoi(serverIni["mysql"]["port"])));

            s->connectSQLServer();

            serverLogger.info("sql加载成功！");
        }
        catch (const std::exception& e)
        {
            serverLogger.critical(e.what());
            serverLogger.critical("sql加载失败！");
            return -1;
        }

        try
        {
            serverLogger.info("服务器监听正在启动，地址：", serverIni["server"]["host"], ":", serverIni["server"]["port"]);
            serverNetwork.setFunctions(std::bind(&SocketFunction::accecptFunction, &serverSocketFunction, std::placeholders::_1),
                                       std::bind(&SocketFunction::receiveFunction, &serverSocketFunction, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                       std::bind(&SocketFunction::closeFunction, &serverSocketFunction, std::placeholders::_1));
            serverNetwork.run(serverIni["server"]["host"], std::stoi(serverIni["server"]["port"]));
        }
        catch (const std::exception& e)
        {
            serverLogger.error(e.what());
            return -1;
        }

        return 0;
    }
}
