#include "socketFunctions.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <Logger.hpp>
#include <Json.h>

#include "definition.hpp"
#include "manager.h"
#include "returnStateMessage.hpp"

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

    asio::awaitable<void> SocketFunction::receiveFunction(asio::ip::tcp::socket& socket, std::string data, std::shared_ptr<qls::DataPackage> pack)
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
    SocketService::SocketService(
        std::shared_ptr<Socket> socket_ptr) :
        m_socket_ptr(socket_ptr),
        m_jsonProcess(-1)
    {
        if (m_socket_ptr.get() == nullptr)
            throw std::logic_error("socket_ptr is nullptr");
    }

    SocketService::~SocketService()
    {
    }

    asio::awaitable<std::shared_ptr<qls::DataPackage>>
        SocketService::async_receive()
    {
        std::string addr = socket2ip(*m_socket_ptr);
        char buffer[8192]{ 0 };

        std::shared_ptr<qls::DataPackage> datapack;
        // 接收数据
        if (!m_package.canRead())
        {
            do
            {
                size_t size = co_await m_socket_ptr->async_read_some(asio::buffer(buffer), asio::use_awaitable);
                // serverLogger.info((std::format("[{}]收到消息: {}", addr, qls::showBinaryData({ buffer, static_cast<size_t>(size) }))));
                m_package.write({ buffer, static_cast<size_t>(size) });
            } while (!m_package.canRead());
        }

        // 检测数据包是否正常
        // 数据包
        datapack = std::shared_ptr<qls::DataPackage>(
            qls::DataPackage::stringToPackage(
                m_package.read()));

        co_return datapack;
    }

    asio::awaitable<size_t> SocketService::async_send(std::string_view data, long long requestID, int type, int sequence)
    {
        std::string out(data);
        auto pack = qls::DataPackage::makePackage(out);
        pack->requestID = requestID;
        pack->sequence = sequence;
        pack->type = type;
        co_return co_await asio::async_write(*m_socket_ptr,
            asio::buffer(pack->packageToString()),
            asio::use_awaitable);
    }

    asio::awaitable<void> SocketService::process(
        std::shared_ptr<Socket> socket_ptr,
        const std::string& data,
        std::shared_ptr<qls::DataPackage> pack)
    {
        if (co_await m_jsonProcess.getLocalUserID() == -1ll && pack->type != 1)
        {
            co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("You have't been logined!")), pack->requestID, 1);
            co_return;
        }

        switch (pack->type)
        {
        case 1:
            // json文本类型
            co_await async_send(qjson::JWriter::fastWrite(
                co_await m_jsonProcess.processJsonMessage(qjson::JParser::fastParse(data))), pack->requestID, 1);
            co_return;
        case 2:
            // 文件类型
            co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("error type")), pack->requestID, 1);// 暂时返回错误
            co_return;
        case 3:
            // 二进制流类型
            co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("error type")), pack->requestID, 1);// 暂时返回错误
            co_return;
        default:
            // 没有这种类型，返回错误
            co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("error type")), pack->requestID, 1);
            co_return;
        }
        co_return;
    }

    void SocketService::setPackageBuffer(const qls::Package& p)
    {
        m_package.setBuffer(p.readBuffer());
    }
    
    asio::awaitable<void> SocketService::echo(Socket socket,
        std::shared_ptr<Network::SocketDataStructure> sds,
        std::chrono::steady_clock::time_point& deadline)
    {
        if (sds.get() == nullptr) throw std::logic_error("sds is nullptr");
        std::shared_ptr<Socket> socket_ptr = std::make_shared<Socket>(std::move(socket));

        SocketService socketService(socket_ptr);

        // 地址
        std::string addr = socket2ip(*socket_ptr);

        try
        {
            long long heart_beat_times = 0;
            auto heart_beat_time_point = std::chrono::steady_clock::now();
            for (;;)
            {
                deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
                auto pack = co_await socketService.async_receive();

                // 判断包是否可用
                if (pack.get() == nullptr)
                {
                    // 数据接收错误
                    serverLogger.error("[", addr, "]", "package is nullptr, auto close connection...");
                    std::error_code ignore_error;
                    socket_ptr->shutdown(ignore_error);
                    co_return;
                }
                else if (pack->type == 4)
                {
                    // 心跳包
                    heart_beat_times++;
                    if (std::chrono::steady_clock::now() - heart_beat_time_point >= std::chrono::seconds(10))
                    {
                        heart_beat_time_point = std::chrono::steady_clock::now();
                        if (heart_beat_times > 10)
                        {
                            serverLogger.error("[", addr, "]", "heart beat too much");
                            co_return;
                        }
                        heart_beat_times = 0;
                    }
                    continue;
                }

                // 成功解密成功接收
                co_await socketService.process(socket_ptr, pack->getData(), pack);
                continue;
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
                serverLogger.error("[", addr, "]", e.code().message());
            }
        }
        catch (const std::exception& e)
        {
            serverLogger.error(std::string(e.what()));
        }

        co_return;
    }
}