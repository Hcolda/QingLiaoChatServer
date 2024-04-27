#include "init.h"

#include <Logger.hpp>
#include <functional>
#include <filesystem>
#include <fstream>

#include "SQLProcess.hpp"
#include "manager.h"
#include "networkEndinass.hpp"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qini::INIObject serverIni;
extern qls::Manager serverManager;

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

            ini["ssl"]["certificate_file"] = "certs.pem";
            ini["ssl"]["password"] = "";
            ini["ssl"]["key_file"] = "key.pem";
            ini["ssl"]["dh_file"] = "dh.pem";

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

        if (qls::isBigEndianness())
            serverLogger.info("服务器的本地端序为大端序");
        else
            serverLogger.info("服务器的本地端序为小端序");

        try
        {
            serverLogger.info("正在读取配置文件...");
            serverIni = Init::readConfig();
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
            if (std::stoll(serverIni["mysql"]["port"]) > 65535)
                throw std::logic_error("ini配置文件 section: mysql, key: port port过大！");

            if (std::stoll(serverIni["mysql"]["port"]) < 0)
                throw std::logic_error("ini配置文件 section: mysql, key: port port过小！");

            // 读取cert && key
            {
                std::ifstream cert(serverIni["ssl"]["certificate_file"]),
                    key(serverIni["ssl"]["key_file"]),
                    dh(serverIni["ssl"]["dh_file"]);

                serverIni["ssl"]["password"];
                if (!cert || !key || !dh)
                    throw std::logic_error("ini配置文件 section: ssl, 无法读取文件！");
            }
            
            serverLogger.info("配置文件读取成功！");
        }
        catch (const std::exception& e)
        {
            serverLogger.error(e.what());
            // Init::createConfig();
            serverLogger.error("请修改配置文件");
            return -1;
        }

        try
        {
            serverLogger.info("正在加载serverManager...");

            serverManager.init();

            serverLogger.info("serverManager加载成功！");
        }
        catch (const std::exception& e)
        {
            serverLogger.critical(e.what());
            serverLogger.critical("serverManager加载失败！");
            return -1;
        }

        try
        {
            serverLogger.info("服务器监听正在启动，地址：", serverIni["server"]["host"], ":", serverIni["server"]["port"]);
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
