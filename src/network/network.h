#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

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
    inline std::string socket2ip(const asio::ssl::stream<tcp::socket>& s);

    inline std::string showBinaryData(const std::string& data);

    class Network
    {
    public:
        struct SocketDataStructure
        {
            // 用于接收数据包
            qls::Package package;
        };

        using acceptFunction = std::function<asio::awaitable<void>(tcp::socket&)>;
        using receiveFunction = std::function<asio::awaitable<void>(tcp::socket&, std::string, std::shared_ptr<qls::DataPackage>)>;
        using closeFunction = std::function<asio::awaitable<void>(tcp::socket&)>;

        Network();
        ~Network();

        /*
        * @brief 设置tls
        * @param callback_handle 用于设置tls的回调函数
        */
        void set_tls_config(std::function<std::shared_ptr<
            asio::ssl::context>()> callback_handle);

        /*
        * @brief 运行network
        * @param host 主机地址
        * @param port 端口
        */
        void run(std::string_view host, unsigned short port);

        void stop();

    private:
        std::string get_password() const;
        awaitable<void> echo(tcp::socket socket);
        awaitable<void> listener();

        std::string                     host_;
        unsigned short                  port_;
        std::unique_ptr<std::thread[]>  threads_;
        const int                       thread_num_;
        asio::io_context                io_context_;
        std::shared_ptr<
            asio::ssl::context>         ssl_context_ptr_;
    };
}

#endif // !NETWORK_HPP
