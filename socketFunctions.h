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

        SocketService(asio::ip::tcp::socket& socket);
        ~SocketService();

        void setAESKeys(const std::string key, const std::string& iv);

        static asio::awaitable<void> echo(asio::ip::tcp::socket socket, const Network::SocketDataStructure& sds);

    private:
        // socket
        asio::ip::tcp::socket&  m_socket;
        // aes
        LocalAES                m_aes;
        // user
        LocalUser               m_user;
        
    };
}