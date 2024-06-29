#include "network.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <Logger.hpp>
#include <Json.h>
#include <Ini.h>

#include "socketFunctions.h"
#include "definition.hpp"
#include "socket.h"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::SocketFunction serverSocketFunction;
extern qini::INIObject serverIni;

qls::Network::Network() :
    port_(55555),
    thread_num_((12 > int(std::thread::hardware_concurrency())
        ? 12 : int(std::thread::hardware_concurrency())))
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

void qls::Network::set_tls_config(
    std::function<std::shared_ptr<
        asio::ssl::context>()> callback_handle)
{
    if (!callback_handle) throw std::logic_error("Tls callback_handle could not be nullptr");
    ssl_context_ptr_ = callback_handle();
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
        serverLogger.error(ERROR_WITH_STACKTRACE(e.what()));
    }
}

std::string qls::Network::get_password() const
{
    return (serverIni["ssl"]["password"]);
}

asio::awaitable<void> qls::Network::echo(asio::ip::tcp::socket origin_socket)
{
    auto executor = co_await asio::this_coro::executor;

    // 加载ssl
    Socket socket(
        std::move(origin_socket), *ssl_context_ptr_);
    // socket加密结构体
    std::shared_ptr<SocketDataStructure> sds = std::make_shared<SocketDataStructure>();
    // string地址，以便数据处理
    std::string addr = socket2ip(socket);

    std::string error_msg;
    try
    {
        serverLogger.info(std::format("[{}]连接至服务器", addr));

        // ssl握手
        co_await socket.async_handshake(asio::ssl::stream_base::server, asio::use_awaitable);

        char data[8192];
        for (;;)
        {
            do
            {
                std::size_t n = co_await socket.async_read_some(asio::buffer(data), use_awaitable);
                // serverLogger.info((std::format("[{}]收到消息: {}", addr, showBinaryData({data, n}))));
                sds->package.write({ data,n });
            } while (!sds->package.canRead());

            while (sds->package.canRead())
            {
                std::shared_ptr<qls::DataPackage> datapack;

                // 检测数据包是否正常
                try
                {
                    // 数据包
                    datapack = std::shared_ptr<qls::DataPackage>(
                        qls::DataPackage::stringToPackage(
                            sds->package.read()));
                    if (datapack->type == 4) continue;
                    if (datapack->getData() != "test")
                        throw std::logic_error("Test error!");
                }
                catch (const std::exception& e)
                {
                    serverLogger.error("[", addr, "]", ERROR_WITH_STACKTRACE(e.what()));
                    std::error_code ignore_error;
                    socket.shutdown(ignore_error);
                    co_return;
                }

                auto execute_function = [](Socket socket,
                    std::shared_ptr<Network::SocketDataStructure> sds) -> asio::awaitable<void> {
                    using namespace asio::experimental::awaitable_operators;
                    auto watchdog = [](std::chrono::steady_clock::time_point & deadline) -> asio::awaitable<void>
                    {
                        asio::steady_timer timer(co_await this_coro::executor);
                        auto now = std::chrono::steady_clock::now();
                        while (deadline > now)
                        {
                            timer.expires_at(deadline);
                            co_await timer.async_wait(use_awaitable);
                            now = std::chrono::steady_clock::now();
                        }
                        throw std::system_error(std::make_error_code(std::errc::timed_out));
                    };
                    std::chrono::steady_clock::time_point deadline;
                    try
                    {
                        co_await(SocketService::echo(std::move(socket), std::move(sds), deadline) && watchdog(deadline));
                    }
                    catch (const std::exception& e)
                    {
                        serverLogger.error(e.what());
                    }
                    co_return;
                    };

                asio::co_spawn(executor, execute_function(std::move(socket), std::move(sds)), asio::detached);
                co_return;
            }
        }
    }
    catch (const std::system_error& e)
    {
        if (e.code().message() == "End of file")
        {
            serverLogger.info(std::format("[{}]与服务器断开连接", addr));
        }
        else
        {
            serverLogger.error(e.code().message());
        }
    }
    catch (const std::exception& e)
    {
        serverLogger.error(ERROR_WITH_STACKTRACE(e.what()));
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

std::string qls::socket2ip(const asio::ssl::stream<tcp::socket>& s)
{
    auto ep = s.lowest_layer().remote_endpoint();
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
