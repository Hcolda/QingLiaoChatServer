#include "socketFunctions.h"

#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <Json.hpp>

#include "definition.hpp"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::SocketFunction serverSocketFunction;
extern qcrypto::pkey::PrivateKey serverPrivateKey;

namespace qls
{
    asio::awaitable<void> SocketFunction::accecptFunction(asio::ip::tcp::socket& socket)
    {
        serverLogger.info(socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), "连接至服务器");
        co_return;
    }

    asio::awaitable<void> SocketFunction::receiveFunction(asio::ip::tcp::socket& socket, std::string data, std::shared_ptr<Network::Package::DataPackage> pack)
    {
        serverLogger.info("接收到数据：", data);
        /*业务逻辑*/

        co_return;
    }

    asio::awaitable<void> SocketFunction::closeFunction(asio::ip::tcp::socket& socket)
    {
        serverLogger.info(socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), "从服务器断开连接");
        co_return;
    }
}
// SocketFunction







// SocketService
namespace qls
{
    SocketService::SocketService(asio::ip::tcp::socket& socket) :
        m_socket(socket)
    {
    }

    SocketService::~SocketService()
    {
    }

    /*
    * @brief 设置aes key iv
    * @param key aes的key
    * @param iv aes的iv
    */
    void SocketService::setAESKeys(const std::string key, const std::string& iv)
    {
        m_aes.AESKey = key;
        m_aes.AESiv = iv;
        m_aes.hasAESKeys = true;
    }

    /*
    * @brief 将socket所有权交到SocketService中
    * @param socket asio::socket类
    * @param sds Network::SocketDataStructure类
    * @return asio协程 asio::awaitable<void>
    */
    asio::awaitable<void> SocketService::echo(asio::ip::tcp::socket socket, const Network::SocketDataStructure& sds)
    {
        SocketService socketService(socket);

        co_return;
    }
}