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
            quqisql::SQLDBProcess s;
            s.setSQLServerInfo("root", "123456", "mysql", "localhost", 3308);
            s.connectSQLServer();
            auto ptr = s.executeQuery("select user_id, user_name from qing_liao_server.users");
            while (ptr->next())
            {
                // Retrieve Values and Print Contacts
                std::cout << "- "
                    << ptr->getString("user_id")
                    << " "
                    << ptr->getString("user_name")
                    << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            serverLogger.critical(e.what());
        }

        return 0;

        try
        {
            serverLogger.info("正在读取配置文件...");
            serverIni = Init::readConfig();
            serverLogger.info("配置文件读取成功！");
        }
        catch (const std::exception& e)
        {
            serverLogger.error(e.what());
            Init::createConfig();
            serverLogger.error("请修改配置文件");
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
