#pragma once

#include <asio.hpp>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <memory>

#include "network.h"

namespace qls
{
    class SocketFunction
    {
    public:
        SocketFunction() = default;
        ~SocketFunction() = default;

        asio::awaitable<void> accecptFunction(asio::ip::tcp::socket& socket);
        asio::awaitable<void> receiveFunction(asio::ip::tcp::socket& socket, std::string data, std::shared_ptr<Network::Package::DataPackage> pack);
        asio::awaitable<void> closeFunction(asio::ip::tcp::socket& socket);
    };

    class SocketService
    {
    public:
        struct LocalAES
        {
            std::string                             AESKey;
            std::string                             AESiv;
            qcrypto::AES<qcrypto::AESMode::CBC_256> AES;
            std::atomic<bool>                       hasAESKeys = false;
        };
        struct LocalUser
        {
            std::string uuid;
            std::string token;
            std::atomic<bool> has_login = false;
        };

        SocketService(std::shared_ptr<asio::ip::tcp::socket> socket_ptr);
        ~SocketService();

        /*
        * @brief 设置aes key iv
        * @param key aes的key
        * @param iv aes的iv
        */
        void setAESKeys(const std::string key, const std::string& iv);

        /*
        * @brief 异步接收数据
        * @param socket
        * @return asio协程 std::string, std::shared_ptr<Network::Package::DataPackage>
        */
        asio::awaitable<std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>>
            async_receive();

        /*
        * @brief 异步发送消息
        * @param socket
        * @param data
        * @param requestID = 0
        * @param type = 0
        * @param sequence = -1
        * @return size 实际发送长度
        */
        asio::awaitable<size_t> async_send(
            std::string_view data,
            long long  requestID = 0,
            int        type = 0,
            int        sequence = -1);

        /*
        * @brief 处理函数
        * @param socket
        * @param data 解密后的数据
        * @param pack 原始数据包
        */
        asio::awaitable<void> proccess(std::shared_ptr<asio::ip::tcp::socket> socket_ptr, const std::string& data, std::shared_ptr<Network::Package::DataPackage> pack);

        /*
        * @brief 设置package
        * @param package
        */
        void setPackageBuffer(const Network::Package& p)
        {
            m_package.setBuffer(p.readBuffer());
        }

        /*
        * @brief 将socket所有权交到SocketService中
        * @param socket asio::socket类
        * @param sds Network::SocketDataStructure类
        * @return asio协程 asio::awaitable<void>
        */
        static asio::awaitable<void> echo(asio::ip::tcp::socket socket, std::shared_ptr<Network::SocketDataStructure> sds);

    private:
        // socket ptr
        std::shared_ptr<asio::ip::tcp::socket> m_socket_ptr;
        // aes
        LocalAES                m_aes;
        // user
        LocalUser               m_user;
        // package
        Network::Package        m_package;
    };
}