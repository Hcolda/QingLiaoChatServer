#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

#include <QuqiCrypto.hpp>

#include "definition.hpp"
#include "package.h"
#include "dataPackage.h"

namespace qls
{
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;

    /*
    * @brief 读取socket地址到string
    * @param socket
    * @return string socket的地址
    */
    inline std::string socket2ip(const asio::ip::tcp::socket& s);

    inline std::string showBinaryData(const std::string& data);

    class Network
    {
    public:
        struct SocketDataStructure
        {
            // 用于接收数据包
            qls::Package package;
            // 用于接收密钥
            std::string uuid;

            // 加密等级 1rsa 2aes 0无
            std::atomic<int> has_encrypt = 0;
            std::string AESKey;
            std::string AESiv;
        };

        using acceptFunction = std::function<asio::awaitable<void>(tcp::socket&)>;
        using receiveFunction = std::function<asio::awaitable<void>(tcp::socket&, std::string, std::shared_ptr<qls::DataPackage>)>;
        using closeFunction = std::function<asio::awaitable<void>(tcp::socket&)>;

        Network();
        ~Network();

        /*
        * @brief 设置函数
        * @param 有新连接时处理函数
        * @param 有数据接收时处理函数
        * @param 有连接主动关闭时的处理函数
        */
        [[deprecated("setFunctions 已经弃用")]] void setFunctions(acceptFunction a, receiveFunction r, closeFunction c);

        /*
        * @brief 运行network
        * @param host 主机地址
        * @param port 端口
        */
        void run(std::string_view host, unsigned short port);

    private:
        awaitable<void> echo(tcp::socket socket);
        awaitable<void> listener();

        std::string                     host_;
        unsigned short                  port_;
        std::unique_ptr<std::thread[]>  threads_;
        const int                       thread_num_;
        acceptFunction                  acceptFunction_;
        receiveFunction                 receiveFunction_;
        closeFunction                   closeFunction_;
    };
}

#endif // !NETWORK_HPP
