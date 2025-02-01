#include "socketFunctions.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <system_error>
#include <logger.hpp>
#include <Json.h>

#include "userid.hpp"
#include "definition.hpp"
#include "manager.h"
#include "returnStateMessage.hpp"
#include "JsonMsgProcess.h"
#include "qls_error.h"

extern Log::Logger serverLogger;
extern qls::Manager serverManager;

namespace qls
{

asio::awaitable<void> SocketFunction::acceptFunction(asio::ip::tcp::socket& socket)
{
    serverLogger.info("[", socket.remote_endpoint().address().to_string(),
        ":", socket.remote_endpoint().port(), "]", "connected to the server");
    co_return;
}

asio::awaitable<void> SocketFunction::receiveFunction(asio::ip::tcp::socket& socket, std::string data, std::shared_ptr<qls::DataPackage> pack)
{
    serverLogger.info("Received data: ", data);
    /* Business logic */

    co_return;
}

asio::awaitable<void> SocketFunction::closeFunction(asio::ip::tcp::socket& socket)
{
    serverLogger.info("[", socket.remote_endpoint().address().to_string(),
        ":", socket.remote_endpoint().port(), "]", "disconnected from the server");
    co_return;
}
}

// SocketFunction

// SocketService
namespace qls
{
struct SocketServiceImpl
{
    // socket ptr
    std::shared_ptr<Connection> m_connection_ptr;
    // JsonMsgProcess
    JsonMessageProcess      m_jsonProcess;
    // package
    qls::Package            m_package;
};

SocketService::SocketService(std::shared_ptr<Connection> connection_ptr) :
    m_impl(std::make_unique<SocketServiceImpl>(connection_ptr, UserID(-1)))
{
    if (!connection_ptr)
        throw std::system_error(qls::qls_errc::null_socket_pointer);
}

SocketService::~SocketService()
{
}

std::shared_ptr<Connection> SocketService::get_connection_ptr() const
{
    return m_impl->m_connection_ptr;
}

asio::awaitable<std::shared_ptr<qls::DataPackage>>
    SocketService::async_receive()
{
    std::string addr = socket2ip(m_impl->m_connection_ptr->socket);
    char buffer[8192]{ 0 };

    std::shared_ptr<qls::DataPackage> datapack;
    // receive data
    if (!m_impl->m_package.canRead()) {
        do {
            std::size_t size = co_await m_impl->m_connection_ptr->socket.async_read_some(asio::buffer(buffer),
                asio::bind_executor(m_impl->m_connection_ptr->strand, asio::use_awaitable));
            m_impl->m_package.write({ buffer, static_cast<std::size_t>(size) });
        } while (!m_impl->m_package.canRead());
    }

    // check data package if it is normal
    datapack = std::shared_ptr<qls::DataPackage>(
        qls::DataPackage::stringToPackage(
            m_impl->m_package.read()));

    co_return datapack;
}

asio::awaitable<std::size_t> SocketService::async_send(std::string_view data, long long requestID, DataPackage::DataPackageType type, int sequence)
{
    std::string out(data);
    auto pack = qls::DataPackage::makePackage(out);
    pack->requestID = requestID;
    pack->sequence = sequence;
    pack->type = type;
    co_return co_await asio::async_write(m_impl->m_connection_ptr->socket,
        asio::buffer(pack->packageToString()),
        asio::bind_executor(m_impl->m_connection_ptr->strand, asio::use_awaitable));
}

asio::awaitable<void> SocketService::process(
    std::shared_ptr<Connection> connection_ptr,
    std::string_view data,
    std::shared_ptr<qls::DataPackage> pack)
{
    if (m_impl->m_jsonProcess.getLocalUserID() == -1ll && pack->type != DataPackage::Text) {
        co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("You haven't logged in!")), pack->requestID, DataPackage::Text);
        co_return;
    }

    switch (pack->type) {
    case DataPackage::Text:
        // json data type
        co_await async_send(qjson::JWriter::fastWrite(
            co_await m_impl->m_jsonProcess.processJsonMessage(qjson::JParser::fastParse(data), *this)), pack->requestID, DataPackage::Text);
        co_return;
    case DataPackage::FileStream:
        // file stream type
        co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("Error type")), pack->requestID, DataPackage::Text); // Temporarily return an error
        co_return;
    case DataPackage::Binary:
        // binary stream type
        co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("Error type")), pack->requestID, DataPackage::Text); // Temporarily return an error
        co_return;
    default:
        // unknown type
        co_await async_send(qjson::JWriter::fastWrite(makeErrorMessage("Error type")), pack->requestID, DataPackage::Text);
        co_return;
    }
    co_return;
}

void SocketService::setPackageBuffer(const qls::Package& p)
{
    m_impl->m_package.setBuffer(p.readBuffer());
}

asio::awaitable<void> SocketService::echo(std::shared_ptr<Connection> connection_ptr,
    std::shared_ptr<Network::SocketDataStructure> sds,
    std::chrono::steady_clock::time_point& deadline)
{
    if (sds.get() == nullptr)
        throw std::logic_error("sds is nullptr");

    SocketService socketService(connection_ptr);

    // get address from socket
    std::string addr = socket2ip(connection_ptr->socket);

    try {
        long long heart_beat_times = 0;
        auto heart_beat_time_point = std::chrono::steady_clock::now();
        for (;;) {
            deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
            auto pack = co_await socketService.async_receive();

            // Determine if the package is available
            if (pack.get() == nullptr) {
                // Data reception error
                serverLogger.error("[", addr, "]", "package is nullptr, auto closing connection...");
                std::error_code ignore_error;
                connection_ptr->socket.shutdown(ignore_error);
                co_return;
            }
            else if (pack->type == DataPackage::HeartBeat) {
                // Heartbeat package
                heart_beat_times++;
                if (std::chrono::steady_clock::now() - heart_beat_time_point >= std::chrono::seconds(10))
                {
                    heart_beat_time_point = std::chrono::steady_clock::now();
                    if (heart_beat_times > 10)
                    {
                        serverLogger.error("[", addr, "]", "too many heartbeats");

                        // remove socket pointer from manager
                        serverManager.removeConnection(connection_ptr);
                        co_return;
                    }
                    heart_beat_times = 0;
                }
                continue;
            }

            // Successfully decrypted and received
            co_await socketService.process(connection_ptr, pack->getData(), pack);
            continue;
        }
    }
    catch (const std::system_error& e) {
        const auto& errc = e.code();
        if (errc.message() == "End of file")
            serverLogger.info(std::format("[{}] disconnected from the server", addr));
        else
            serverLogger.error('[', addr, ']', '[', errc.category().name(), ']', errc.message());
    }
    catch (const std::exception& e) {
        serverLogger.error(std::string(e.what()));
    }

    // remove socket pointer from manager
    serverManager.removeConnection(connection_ptr);
    co_return;
}

} // namespace qls
