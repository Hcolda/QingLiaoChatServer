#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

#include "definition.hpp"
#include "package.h"
#include "dataPackage.h"
#include "socket.h"

namespace qls
{

using asio::ip::tcp;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;

/**
 * @brief Converts a socket's address to a string representation.
 * @param s The socket.
 * @return The string representation of the socket's address.
 */
inline std::string socket2ip(const qls::Socket& s);

/**
 * @brief Displays binary data as a string.
 * @param data The binary data.
 * @return The string representation of the binary data.
 */
inline std::string showBinaryData(const std::string& data);

/**
 * @class Network
 * @brief Manages network operations including connection handling and data transmission.
 */
class Network final
{
public:
    /**
     * @struct SocketDataStructure
     * @brief Structure to hold data associated with a socket.
     */
    struct SocketDataStructure
    {
        qls::Package package; ///< Package used to receive data.
    };

    using acceptFunction = std::function<asio::awaitable<void>(tcp::socket&)>;
    using receiveFunction = std::function<asio::awaitable<void>(tcp::socket&, std::string, std::shared_ptr<qls::DataPackage>)>;
    using closeFunction = std::function<asio::awaitable<void>(tcp::socket&)>;

    Network();
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    ~Network();

    /**
     * @brief Sets the TLS configuration.
     * @param callback_handle A callback function to configure TLS.
     */
    void set_tls_config(std::function<std::shared_ptr<asio::ssl::context>()> callback_handle);

    /**
     * @brief Runs the network.
     * @param host The host address.
     * @param port The port number.
     */
    void run(std::string_view host, unsigned short port);

    /**
     * @brief Stops the network operations.
     */
    void stop();

private:
    /**
     * @brief Retrieves the password for the SSL context.
     * @return The password string.
     */
    std::string get_password() const;

    /**
     * @brief Handles echo functionality for a socket.
     * @param socket The socket.
     * @return An awaitable task.
     */
    awaitable<void> echo(tcp::socket socket);

    /**
     * @brief Listens for incoming connections.
     * @return An awaitable task.
     */
    awaitable<void> listener();

    std::string                         host_; ///< Host address.
    unsigned short                      port_; ///< Port number.
    std::unique_ptr<std::thread[]>      threads_; ///< Thread pool for handling connections.
    const int                           thread_num_; ///< Number of threads.
    asio::io_context                    io_context_; ///< IO context for ASIO.
    std::shared_ptr<asio::ssl::context> ssl_context_ptr_; ///< Shared pointer to the SSL context.
};

} // namespace qls

#endif // !NETWORK_HPP
