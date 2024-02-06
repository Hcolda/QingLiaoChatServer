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

qls::Network::Network() :
    port_(55555),
    thread_num_((12 > int(std::thread::hardware_concurrency())
        ? int(std::thread::hardware_concurrency()) : 12)),
    acceptFunction_([](tcp::socket&) -> asio::awaitable<void> {co_return; }),
    receiveFunction_([](tcp::socket&,
        std::string, std::shared_ptr<qls::DataPackage>
        ) -> asio::awaitable<void> {co_return; }),
    closeFunction_([](tcp::socket&) -> asio::awaitable<void> {co_return; })
    {
        // 等thread_num初始化之后才能申请threads内存
        threads_ = std::unique_ptr<std::thread[]>(new std::thread[size_t(thread_num_) + 1]{});
    }

qls::Network::~Network()
{
    for (int i = 0; i < thread_num_; i++)
    {
        if (threads_[i].joinable())
            threads_[i].join();
    }
}

void qls::Network::setFunctions(acceptFunction a, receiveFunction r, closeFunction c)
{
    acceptFunction_ = std::move(a);
    receiveFunction_ = std::move(r);
    closeFunction_ = std::move(c);
}

void qls::Network::run(std::string_view host, unsigned short port)
{
    host_ = host;
    port_ = port;

    try
    {
        asio::io_context io_context;

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        for (int i = 0; i < thread_num_; i++)
        {
            threads_[i] = std::thread([&]() {
                co_spawn(io_context, listener(), detached);
                io_context.run();
                });
        }

        for (int i = 0; i < thread_num_; i++)
        {
            if (threads_[i].joinable())
                threads_[i].join();
        }
    }
    catch (const std::exception& e)
    {
        std::printf("Exception: %s\n", e.what());
    }
}

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
        serverLogger.info(std::format("[{}]连接至服务器", addr));
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
                serverLogger.info((std::format("[{}]收到消息: {}", addr, showBinaryData({data, n}))));
                sds->package.write({ data,n });
            } while (!sds->package.canRead());

            while (sds->package.canRead())
            {
                std::shared_ptr<qls::DataPackage> datapack;

                // 检测数据包是否正常
                {
                    // 数据包
                    try
                    {
                        // 数据包
                        datapack = std::shared_ptr<qls::DataPackage>(
                            qls::DataPackage::stringToPackage(
                                sds->package.read()));
                        if (datapack->getData() != "test")
                            throw std::logic_error("Test error!");
                    }
                    catch (const std::exception& e)
                    {
                        serverLogger.error("[", addr, "]", ERROR_WITH_STACKTRACE(e.what()));
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

asio::awaitable<void> qls::Network::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { asio::ip::address::from_string(host_), port_ });
    for (;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

std::string qls::socket2ip(const asio::ip::tcp::socket& s)
{
    auto ep = s.remote_endpoint();
    return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
}

std::string qls::showBinaryData(const std::string& data)
{
    auto isShowableCharactor = [](unsigned char ch) -> bool {
        return 32 <= ch && ch <= 126;
        };

    std::string result;

    for (const auto& i : data)
    {
        if (isShowableCharactor(static_cast<unsigned char>(i)))
        {
            result += i;
        }
        else
        {
            std::string hex;
            int locch = static_cast<unsigned char>(i);
            while (locch)
            {
                if (locch % 16 < 10)
                {
                    hex += ('0' + (locch % 16));
                    locch /= 16;
                    continue;
                }
                switch (locch % 16)
                {
                case 10:
                    hex += 'a';
                    break;
                case 11:
                    hex += 'b';
                    break;
                case 12:
                    hex += 'c';
                    break;
                case 13:
                    hex += 'd';
                    break;
                case 14:
                    hex += 'e';
                    break;
                case 15:
                    hex += 'f';
                    break;
                }
                locch /= 16;
            }

            //result += "\\x" + (hex.size() == 1 ? "0" + hex : hex);
            if (hex.empty())
            {
                result += "\\x00";
            }
            else if (hex.size() == 1)
            {
                result += "\\x0" + hex;
            }
            else
            {
                result += "\\x" + hex;
            }
        }
    }

    return result;
}
