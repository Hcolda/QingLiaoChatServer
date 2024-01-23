#include "socketFunctions.h"

#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <Json.h>

#include "definition.hpp"
#include "websiteFunctions.hpp"
#include "manager.h"

extern Log::Logger serverLogger;
extern qls::Manager serverManager;

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
    SocketService::SocketService(std::shared_ptr<asio::ip::tcp::socket> socket_ptr) :
        m_socket_ptr(socket_ptr),
        m_jsonProcess(-1)
    {
        if (m_socket_ptr.get() == nullptr)
            throw std::logic_error("socket_ptr is nullptr");
    }

    SocketService::~SocketService()
    {
    }

    
    void SocketService::setAESKeys(const std::string key, const std::string& iv)
    {
        m_aes.AESKey = key;
        m_aes.AESiv = iv;
        if (key.size() == 32 && iv.size() == 16)
            m_aes.hasAESKeys = true;
        else
            m_aes.hasAESKeys = false;
    }

    void SocketService::setUUID(const std::string& uuid)
    {
        m_user.uuid = uuid;
    }

    asio::awaitable<std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>>
        SocketService::async_receive()
    {
        std::string addr = socket2ip(*m_socket_ptr);
        char buffer[8192]{ 0 };
        // 接收数据
        if (!m_package.canRead())
        {
            do
            {
                size_t size = co_await m_socket_ptr->async_read_some(asio::buffer(buffer), asio::use_awaitable);
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

        std::string out = datapack->getData(datapack);

        // 先不加密
        if (m_aes.hasAESKeys)
        {
            if (!m_aes.AES.encrypt(datapack->getData(datapack), out, m_aes.AESKey, m_aes.AESiv, false))
            {
                serverLogger.warning("[", addr, "]", ERROR_WITH_STACKTRACE("encrypt failed"));
                co_return std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>{std::string(), datapack};
            }
        }
        co_return std::pair<std::string, std::shared_ptr<Network::Package::DataPackage>>{out, datapack};
    }

    asio::awaitable<size_t> SocketService::async_send(std::string_view data, long long requestID, int type, int sequence)
    {
        std::string out(data);
        //先不加密
        if (m_aes.hasAESKeys)
        {
            m_aes.AES.encrypt(data, out, m_aes.AESKey, m_aes.AESiv, true);
        }
        auto pack = Network::Package::DataPackage::makePackage(out);
        pack->requestID = requestID;
        pack->sequence = sequence;
        pack->type = type;
        co_return co_await m_socket_ptr->async_send(asio::buffer(pack->packageToString(pack)), asio::use_awaitable);
    }

    asio::awaitable<void> SocketService::process(std::shared_ptr<asio::ip::tcp::socket> socket_ptr,
        const std::string& data,
        std::shared_ptr<Network::Package::DataPackage> pack)
    {
        //if (!this->m_jsonProcess)
        //{
        //    // 如果json process没有加载
        //    // 获取用户的id并创建json process

        //    // long long user_id = WebFunction::getUserID(this->m_user.uuid);
        //    this->m_jsonProcess = std::make_shared<JsonMessageProcess>(-1);
        //}

        if (this->m_jsonProcess.getLocalUserID() == -1 && pack->type != 1)
            co_await this->async_send(qjson::JWriter::fastWrite(JsonMessageProcess::makeErrorMessage("You have't been logined!")), pack->requestID, 1);

        switch (pack->type)
        {
        case 1:
        {
            // json文本类型
            co_await this->async_send(qjson::JWriter::fastWrite(this->m_jsonProcess.processJsonMessage(data)), pack->requestID, 1);
        }
            break;
        case 2:
        {
            // 文件类型
            co_await this->async_send(qjson::JWriter::fastWrite(JsonMessageProcess::makeErrorMessage("error type")), pack->requestID, 1);// 暂时返回错误
        }
        break;
        case 3:
        {
            // 二进制流类型
            co_await this->async_send(qjson::JWriter::fastWrite(JsonMessageProcess::makeErrorMessage("error type")), pack->requestID, 1);// 暂时返回错误
        }
        break;
        default:
        {
            // 没有这种类型，返回错误
            co_await this->async_send(qjson::JWriter::fastWrite(JsonMessageProcess::makeErrorMessage("error type")), pack->requestID, 1);
        }
            break;
        }
        co_return;
    }
    
    asio::awaitable<void> SocketService::echo(asio::ip::tcp::socket socket, std::shared_ptr<Network::SocketDataStructure> sds)
    {
        if (sds.get() == nullptr) throw std::logic_error("sds is nullptr");
        std::shared_ptr<asio::ip::tcp::socket> socket_ptr = std::make_shared<asio::ip::tcp::socket>(std::move(socket));

        SocketService socketService(socket_ptr);
        socketService.setAESKeys(sds->AESKey, sds->AESiv);
        socketService.setPackageBuffer(sds->package);
        socketService.setUUID(sds->uuid);

        // 地址
        std::string addr = socket2ip(*socket_ptr);

        try
        {
            for (;;)
            {
                auto [data, pack] = co_await socketService.async_receive();

                // 判断包是否可用
                if (pack.get() == nullptr)
                {
                    // 数据接收错误
                    serverLogger.error("[", addr, "]", "package is nullptr, auto close connection...");
                    socket_ptr->close();
                    co_return;
                }
                else if (data.empty())
                {
                    // 数据成功接收但是无法aes解密
                    serverLogger.warning("[", addr, "]", "data is invalid");
                    continue;
                }

                // 成功解密成功接收
                co_await socketService.process(socket_ptr, data, pack);
                continue;
            }
        }
        catch (const std::exception& e)
        {
            if (!strcmp(e.what(), "End of file"))
            {
                serverLogger.info(std::format("[{}]与服务器断开连接", addr));
            }
            else
            {
                serverLogger.warning(std::string(e.what()));
            }
        }

        co_return;
    }
}