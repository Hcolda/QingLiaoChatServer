#ifndef SOCKET_FUNCTIONS_H
#define SOCKET_FUNCTIONS_H

#include <asio.hpp>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <memory>

#include "dataPackage.h"
#include "package.h"
#include "network.h"
#include "socket.h"
#include "connection.hpp"

namespace qls
{

struct SocketServiceImpl;

class SocketService final
{
public:
    SocketService(std::shared_ptr<Connection> connection_ptr);
    ~SocketService();

    /**
    * @brief Get the socket pointer
    * @return Connection pointer
    */
    std::shared_ptr<Connection> get_connection_ptr() const;

    /**
    * @brief Asynchronously receive data
    * @return ASIO coroutine returning std::shared_ptr<Network::Package::DataPackage>
    */
    asio::awaitable<std::shared_ptr<qls::DataPackage>> async_receive();

    /**
    * @brief Asynchronously send a message
    * @param socket
    * @param data
    * @param requestID = 0
    * @param type = 0
    * @param sequence = -1
    * @return size Actual length sent
    */
    asio::awaitable<std::size_t> async_send(
        std::string_view                data,
        long long                       requestID = 0,
        DataPackage::DataPackageType    type = DataPackage::Unknown,
        int                             sequence = -1);

    /**
    * @brief Process function
    * @param connection_ptr Connection pointer
    * @param data Decrypted data
    * @param pack Original data packet
    */
    asio::awaitable<void> process(
        std::shared_ptr<Connection> connection_ptr,
        std::string_view data,
        std::shared_ptr<qls::DataPackage> pack);

    /**
    * @brief Set the package buffer
    * @param package
    */
    void setPackageBuffer(const qls::Package& p);

    /**
    * @brief Transfer socket ownership to SocketService
    * @param connection_ptr Connection pointer
    * @param sds Network::SocketDataStructure class
    * @param deadline the deadline to close the connection
    * @return ASIO coroutine asio::awaitable<void>
    */
    static asio::awaitable<void> echo(
        std::shared_ptr<Connection> connection_ptr,
        std::shared_ptr<Network::SocketDataStructure> sds,
        std::chrono::steady_clock::time_point& deadline);

private:
    std::unique_ptr<SocketServiceImpl> m_impl;
};

} // namespace qls

#endif // !SOCKET_FUNCTIONS_H
