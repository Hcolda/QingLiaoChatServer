#include "network.h"

#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <Json.h>

#include "socketFunctions.h"
#include "definition.hpp"
#include "websiteFunctions.hpp"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::SocketFunction serverSocketFunction;

asio::awaitable<void> qls::Network::echo(asio::ip::tcp::socket socket)
{
    auto executor = co_await asio::this_coro::executor;

    // socket加密结构体
    std::shared_ptr<SocketDataStructure> sds = std::make_shared<SocketDataStructure>();
    // string地址，以便数据处理
    std::string addr = socket2ip(socket);

    // 异步发送消息函数
    /*auto async_local_send = [&](const std::string& data) -> asio::awaitable<void> {
        if (sds.has_encrypt != 2) throw std::runtime_error("cant send");

        co_return;
        };*/

    bool has_closed = false;
    std::string error_msg;
    try
    {
        co_await acceptFunction_(socket);

        // 发送pem密钥给客户端
        /*{
            std::string pem;
            qcrypto::PEM::PEMWritePublicKey(qcrypto::pkey::PublicKey(serverPrivateKey), pem);
            auto pack = Network::Package::DataPackage::makePackage(pem);

            co_await socket.async_send(asio::buffer(pack->packageToString(pack)), asio::use_awaitable);
        }*/

        char data[8192];
        for (;;)
        {
            do
            {
                std::size_t n = co_await socket.async_read_some(asio::buffer(data), use_awaitable);
                sds->package.write({ data,n });
            } while (!sds->package.canRead());

            while (sds->package.canRead())
            {
                std::shared_ptr<Network::Package::DataPackage> datapack;

                // 检测数据包是否正常
                {
                    // 数据包
                    try
                    {
                        // 数据包
                        datapack = std::shared_ptr<Network::Package::DataPackage>(
                            Network::Package::DataPackage::stringToPackage(
                                sds->package.read()));
                    }
                    catch (const std::exception& e)
                    {
                        serverLogger.warning("[", addr, "]", ERROR_WITH_STACKTRACE(e.what()));
                        closeFunction_(socket);
                        socket.close();
                        co_return;
                    }
                }

                // 加密检测
                //{
                //    // 加密检测函数 lambda（新）
                //    auto encryptFunction_2 = [&]() -> asio::awaitable<int> {
                //        
                //        if (sds->has_encrypt == 0)
                //        {
                //            sds->uuid = datapack->getData(datapack);

                //            WebFunction web;
                //            auto [key, iv] = web.getAESKey(sds->uuid);

                //            std::string key1, iv1;
                //            qcrypto::Base64::encrypt(key, key1, false);
                //            qcrypto::Base64::encrypt(iv, iv1, false);

                //            sds->AESKey = key1;
                //            sds->AESiv = iv1;

                //            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
                //            std::string out;
                //            aes.encrypt("hello client", out, sds->AESKey, sds->AESiv, true);
                //            auto sendpack = Package::DataPackage::makePackage(out);
                //            sendpack->requestID = datapack->requestID;
                //            co_await socket.async_send(asio::buffer(sendpack->packageToString(sendpack)), asio::use_awaitable);

                //            sds->has_encrypt = 1;
                //            co_return 1;
                //        }
                //        else if (sds->has_encrypt == 1)
                //        {
                //            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
                //            std::string out;
                //            if (!aes.encrypt(datapack->getData(datapack), out, sds->AESKey, sds->AESiv, false) || out != "hello server")
                //            {
                //                serverLogger.warning("[", addr, "]", ERROR_WITH_STACKTRACE("decrypt error in file "));
                //                closeFunction_(socket);
                //                socket.close();
                //                co_return 0;
                //            }

                //            sds->has_encrypt = 2;
                //            co_return 2;
                //        }
                //        else co_return 2;
                //        };

                //    // 1是加密到一半 2是完全加密
                //    int code = co_await encryptFunction_2();
                //    switch (code)
                //    {
                //    case 0:
                //        co_return;
                //    case 1:
                //        continue;
                //    case 2:
                //    {
                //        // 将socket所有权交给新类
                //        asio::co_spawn(executor, SocketService::echo(std::move(socket), std::move(sds)), asio::detached);
                //        co_return;
                //    }
                //    default:
                //        break;
                //    }
                //}

                asio::co_spawn(executor, SocketService::echo(std::move(socket), std::move(sds)), asio::detached);
                co_return;
            }
        }
    }
    catch (std::exception& e)
    {
        if (!strcmp(e.what(), "End of file"))
        {
            has_closed = true;
        }
        else
        {
            serverLogger.warning(ERROR_WITH_STACKTRACE(e.what()));
        }
    }

    if (has_closed)
    {
        co_await closeFunction_(socket);
    }
    co_return;
}
