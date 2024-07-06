#include "init.h"

#include <Logger.hpp>
#include <functional>
#include <filesystem>
#include <fstream>

#include "SQLProcess.hpp"
#include "manager.h"
#include "networkEndinass.hpp"
#include "input.h"

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
        serverLogger.info("服务器Log系统启动成功!");

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
            serverLogger.error(std::string(e.what()));
            Init::createConfig();
            serverLogger.error("请修改配置文件");
            return -1;
        }

        try
        {
            if (std::stoll(serverIni["mysql"]["port"]) > 65535)
                throw std::logic_error("ini配置文件 section: mysql, key: port port过大!");

            if (std::stoll(serverIni["mysql"]["port"]) < 0)
                throw std::logic_error("ini配置文件 section: mysql, key: port port过小!");

            // 读取cert && key
            {
                {
                    std::ifstream cert(serverIni["ssl"]["certificate_file"]),
                        key(serverIni["ssl"]["key_file"]),
                        dh(serverIni["ssl"]["dh_file"]);

                    serverIni["ssl"]["password"];
                    if (!cert || !key)
                        throw std::logic_error("ini配置文件 section: ssl, 无法读取文件!");
                }

                serverLogger.info("ceritificate_file路径: ", serverIni["ssl"]["certificate_file"]);
                serverLogger.info("密码: ", (serverIni["ssl"]["password"].empty() ? "空" : serverIni["ssl"]["password"]));
                serverLogger.info("key_file路径: ", serverIni["ssl"]["key_file"]);
                serverLogger.info("dh_file路径: ", serverIni["ssl"]["dh_file"]);

                serverNetwork.set_tls_config([](){
                    std::shared_ptr<asio::ssl::context> ssl_context =
                            std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);

                    // 设置ssl参数
                    ssl_context->set_options(
                        asio::ssl::context::default_workarounds
                        | asio::ssl::context::no_sslv2
                        | asio::ssl::context::no_sslv3
                        | asio::ssl::context::no_tlsv1
                        | asio::ssl::context::no_tlsv1_1
                        | asio::ssl::context::single_dh_use
                    );

                    // 配置ssl context
                    if (!serverIni["ssl"]["password"].empty())
                        ssl_context->set_password_callback(std::bind([](){ return serverIni["ssl"]["password"]; }));
                    ssl_context->use_certificate_chain_file(serverIni["ssl"]["certificate_file"]);
                    ssl_context->use_private_key_file(serverIni["ssl"]["key_file"], asio::ssl::context::pem);
                    return ssl_context;
                });
                serverLogger.info("设置TLS成功");
            }
            
            serverLogger.info("配置文件读取成功！");
        }
        catch (const std::exception& e)
        {
            serverLogger.error(std::string(e.what()));
            // Init::createConfig();
            serverLogger.error("请修改配置文件");
            return -1;
        }

        try
        {
            serverLogger.info("正在加载serverManager...");

            serverManager.init();

            serverLogger.info("serverManager加载成功!");
        }
        catch (const std::exception& e)
        {
            serverLogger.critical(std::string(e.what()));
            serverLogger.critical("serverManager加载失败!");
            return -1;
        }

        try
        {
            serverLogger.info("服务器命令行启动...");
            std::thread([](){
                Input input;
                std::string command;
                while (true)
                {
                    std::cin >> command;
                    if (!input.input(command))
                        break;
                }
            }).detach();
            
            serverLogger.info("服务器监听正在启动，地址：", serverIni["server"]["host"], ":", serverIni["server"]["port"]);
            serverNetwork.run(serverIni["server"]["host"], std::stoi(serverIni["server"]["port"]));
            
        }
        catch (const std::exception& e)
        {
            serverLogger.error(std::string(e.what()));
            return -1;
        }

        return 0;
    }
}
