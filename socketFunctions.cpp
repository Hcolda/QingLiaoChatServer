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
        serverLogger.info("[", socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), "]", "连接至服务器");
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
        serverLogger.info("[", socket.remote_endpoint().address().to_string(),
            ":", socket.remote_endpoint().port(), "]", "从服务器断开连接");
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

    
    void SocketService::setAESKeys(const std::string key, const std::string& iv)
    {
        m_aes.AESKey = key;
        m_aes.AESiv = iv;
        m_aes.hasAESKeys = true;
    }

    asio::awaitable<std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>>
        SocketService::async_receive(asio::ip::tcp::socket& socket)
    {
        std::string addr = socket2ip(socket);
        char buffer[8192]{ 0 };
        // 接收数据
        if (!m_package.canRead())
        {
            do
            {
                size_t size = co_await socket.async_read_some(asio::buffer(buffer), asio::use_awaitable);
                m_package.write({ buffer, size });
            } while (!m_package.canRead());
        }

        std::shared_ptr<Network::Package::DataPackage> datapack;

        // 检测数据包是否正常
        {
            // 数据包
            try
            {
                datapack = std::shared_ptr<Network::Package::DataPackage>(
                    Network::Package::DataPackage::stringToPackage(
                        m_package.read()));
            }
            catch (const std::exception& e)
            {
                serverLogger.warning("[", addr, "]", ERROR_WITH_STACKTRACE(e.what()));
                co_return std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>{std::string(), nullptr};
            }
        }

        std::string out;
        if (!m_aes.AES.encrypt(datapack->getData(datapack), out, m_aes.AESKey, m_aes.AESiv, false))
        {
            serverLogger.warning("[", addr, "]", ERROR_WITH_STACKTRACE("encrypt failed"));
            co_return std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>{std::string(), datapack};
        }
        co_return std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>{out, datapack};
    }

    asio::awaitable<size_t> SocketService::async_send(asio::ip::tcp::socket& socket, std::string_view data, long long requestID, int type, int sequence)
    {
        std::string out;
        m_aes.AES.encrypt(data, out, m_aes.AESKey, m_aes.AESiv, true);
        auto pack = Network::Package::DataPackage::makePackage(out);
        pack->requestID = requestID;
        pack->sequence = sequence;
        pack->type = type;
        co_return co_await socket.async_send(asio::buffer(pack->packageToString(pack)), asio::use_awaitable);
    }
    
    asio::awaitable<void> SocketService::echo(asio::ip::tcp::socket socket, std::shared_ptr<Network::SocketDataStructure> sds)
    {
        if (sds.get() == nullptr) throw std::logic_error("sds is nullptr");
        SocketService socketService(socket);
        socketService.setAESKeys(sds->AESKey, sds->AESiv);
        socketService.setPackageBuffer(sds->package);


        // 地址
        std::string addr = socket2ip(socket);

        for (;;)
        {
            auto [data, pack] = co_await socketService.async_receive(socket);

            // 判断包是否可用
            if (pack.get() == nullptr)
            {
                // 数据接收错误
                serverLogger.error("[", addr, "]", "package is nullptr, auto close connection...");
                socket.close();
                co_return;
            }
            else if (data.empty())
            {
                // 数据成功接收但是无法aes解密
                serverLogger.warning("[", addr, "]", "data is invalid");
                continue;
            }

            // 成功解密成功接收
            serverLogger.info("[", addr, "]", "send msg: ", data);
            continue;
        }

        co_return;
    }
}