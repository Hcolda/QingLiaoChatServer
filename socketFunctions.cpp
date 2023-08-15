#include "socketFunctions.h"

#include <Logger.hpp>
#include <QuqiCrypto.hpp>

#include "network.hpp"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::SocketFunction serverSocketFunction;
extern qcrypto::pkey::PrivateKey serverPrivateKey;

/*
* @brief 读取socket地址到string
* @param socket
* @return string socket的地址
*/
static inline std::string socket2ip(const asio::ip::tcp::socket& s)
{
    auto ep = s.remote_endpoint();
    return ep.address().to_string() + std::to_string(int(ep.port()));
}

namespace qls
{
    asio::awaitable<void> SocketFunction::accecptFunction(asio::ip::tcp::socket& socket)
    {
        // 发送pem密钥给客户端
        {
            std::string pem;
            qcrypto::PEM::PEMWritePublicKey(qcrypto::pkey::PublicKey(serverPrivateKey), pem);

            co_await socket.async_send(asio::buffer(pem), asio::use_awaitable);
        }

        // 给socket加入map
        {
            std::lock_guard<std::shared_mutex> lock(m_shared_mutex);
            m_socketMap[socket2ip(socket)].has_encrypt = 0;
        }

        serverLogger.info(socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), "连接至服务器");
        co_return;
    }

    asio::awaitable<void> SocketFunction::receiveFunction(asio::ip::tcp::socket& socket, std::string data)
    {
        /*此处还没进行更改代码逻辑*/
        serverLogger.info(socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), ": ", data.substr(4));
        co_await socket.async_send(asio::buffer(data), asio::use_awaitable);
        co_return;
    }

    asio::awaitable<void> SocketFunction::closeFunction(asio::ip::tcp::socket& socket)
    {
        // 在map中删除socket
        {
            std::lock_guard<std::shared_mutex> lock(m_shared_mutex);
            if (m_socketMap.find(socket2ip(socket)) != m_socketMap.end())
            {
                m_socketMap.erase(m_socketMap.find(socket2ip(socket)));
            }
        }

        serverLogger.info(socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), "从服务器断开连接");
        co_return;
    }
}