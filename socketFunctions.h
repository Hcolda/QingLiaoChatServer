#pragma once

#include <asio.hpp>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>

namespace qls
{
    class SocketFunction
    {
    public:
        SocketFunction() = default;
        ~SocketFunction() = default;

        asio::awaitable<void> accecptFunction(asio::ip::tcp::socket& socket);
        asio::awaitable<void> receiveFunction(asio::ip::tcp::socket& socket, std::string data);
        asio::awaitable<void> closeFunction(asio::ip::tcp::socket& socket);

    protected:
        struct SocketDataStructure
        {
            // 加密等级 1rsa 2aes 0无
            std::atomic<int> has_encrypt = 0;

            //要到2等级才能使用以下数据

            // uuid
            std::string uuid;
            // token
            std::string token;
        };

    private:
        std::unordered_map<std::string, SocketDataStructure>    m_socketMap;
        std::shared_mutex                                       m_shared_mutex;
    };
}