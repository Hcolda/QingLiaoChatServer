#include "network.h"

#include <memory_resource>
#include <asio/experimental/awaitable_operators.hpp>
#include <logger.hpp>
#include <system_error>
#include <Json.h>
#include <Ini.h>

#include "socketFunctions.h"
#include "definition.hpp"
#include "socket.h"
#include "manager.h"
#include "qls_error.h"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::Manager serverManager;
extern qls::SocketFunction serverSocketFunction;
extern qini::INIObject serverIni;

namespace qls {
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;
}

qls::Network::Network() :
    m_port(55555),
    m_thread_num((12 > static_cast<int>(std::thread::hardware_concurrency())
        ? 12 : static_cast<int>(std::thread::hardware_concurrency())))
    {
        m_threads = std::make_unique<std::thread[]>(static_cast<std::size_t>(m_thread_num));
    }

qls::Network::~Network()
{
    for (int i = 0; i < m_thread_num; i++) {
        if (m_threads[i].joinable())
            m_threads[i].join();
    }
}

void qls::Network::setTlsConfig(
    std::function<std::shared_ptr<
        asio::ssl::context>()> callback_handle)
{
    if (!callback_handle)
        throw std::system_error(qls_errc::null_tls_callback_handle);
    m_ssl_context_ptr = callback_handle();
    if (!m_ssl_context_ptr)
        throw std::system_error(qls_errc::null_tls_context);
}

void qls::Network::run(std::string_view host, unsigned short port)
{
    m_host = host;
    m_port = port;

    // Check if SSL context ptr is null
    if (!m_ssl_context_ptr)
        throw std::system_error(qls_errc::null_tls_context);

    try {
        asio::signal_set signals(m_io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { m_io_context.stop(); });

        for (int i = 0; i < m_thread_num; i++) {
            m_threads[i] = std::thread([&]() {
                co_spawn(m_io_context, listener(), detached);
                m_io_context.run();
                });
        }

        for (int i = 0; i < m_thread_num; i++) {
            if (m_threads[i].joinable())
                m_threads[i].join();
        }
    }
    catch (const std::exception& e) {
        serverLogger.error(ERROR_WITH_STACKTRACE(e.what()));
    }
}

void qls::Network::stop()
{
    m_io_context.stop();
}

static std::pmr::synchronized_pool_resource socket_sync_pool;

asio::awaitable<void> qls::Network::echo(asio::ip::tcp::socket origin_socket)
{
    auto executor = co_await asio::this_coro::executor;

    // Load SSL socket pointer
    std::shared_ptr<Socket> socket_ptr = std::allocate_shared<Socket>(
        std::pmr::polymorphic_allocator<Socket>(&socket_sync_pool),
        std::move(origin_socket), *m_ssl_context_ptr);
    // Socket encrypted structure
    std::shared_ptr<SocketDataStructure> sds = std::make_shared<SocketDataStructure>();
    // String address for data processing
    std::string addr = socket2ip(*socket_ptr);

    // register the socket
    serverManager.registerSocket(socket_ptr);

    try {
        serverLogger.info(std::format("[{}] connected to the server", addr));

        // SSL handshake
        co_await socket_ptr->async_handshake(asio::ssl::stream_base::server, asio::use_awaitable);

        char data[8192] {0};
        for (;;) {
            do {
                std::size_t n = co_await socket_ptr->async_read_some(asio::buffer(data), use_awaitable);
                // serverLogger.info((std::format("[{}] received message: {}", addr, showBinaryData({data, n}))));
                sds->package.write({ data, n });
            } while (!sds->package.canRead());

            while (sds->package.canRead()) {
                std::shared_ptr<qls::DataPackage> datapack;

                // Check if the data package is normal
                try {
                    // Data package
                    datapack = std::shared_ptr<qls::DataPackage>(
                        qls::DataPackage::stringToPackage(
                            sds->package.read()));
                    if (datapack->type == DataPackage::HeartBeat) continue;
                    if (datapack->getData() != "test")
                        throw std::system_error(qls_errc::connection_test_failed);
                } catch (const std::exception& e) {
                    serverLogger.error("[", addr, "]", ERROR_WITH_STACKTRACE(e.what()));
                    std::error_code ignore_error;
                    socket_ptr->shutdown(ignore_error);
                    co_return;
                }

                auto executeFunction = [addr](std::shared_ptr<Socket> socket_ptr,
                    std::shared_ptr<Network::SocketDataStructure> sds) -> asio::awaitable<void> {
                    using namespace asio::experimental::awaitable_operators;
                    auto watchdog = [addr](std::chrono::steady_clock::time_point & deadline) -> asio::awaitable<void> {
                        asio::steady_timer timer(co_await this_coro::executor);
                        auto now = std::chrono::steady_clock::now();
                        while (deadline > now) {
                            timer.expires_at(deadline);
                            co_await timer.async_wait(use_awaitable);
                            now = std::chrono::steady_clock::now();
                        }
                        throw std::system_error(make_error_code(std::errc::timed_out));
                    };
                    std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
                    try {
                        co_await (SocketService::echo(socket_ptr, std::move(sds), deadline) && watchdog(deadline));
                    } catch (const std::system_error& e) {
                        const auto& errc = e.code();
                        if (errc.message() == "End of file")
                            serverLogger.info(std::format("[{}] disconnected from the server", addr));
                        else
                            serverLogger.error('[', errc.category().name(), ']', errc.message());
                    } catch (const std::exception& e) {
                        serverLogger.error(std::string(e.what()));
                    }
                    serverManager.removeSocket(socket_ptr);
                    co_return;
                    };

                asio::co_spawn(executor, executeFunction(std::move(socket_ptr), std::move(sds)), asio::detached);
                co_return;
            }
        }
    }
    catch (const std::system_error& e) {
        const auto& errc = e.code();
        if (errc.message() == "End of file")
            serverLogger.info(std::format("[{}] disconnected from the server", addr));
        else
            serverLogger.error('[', errc.category().name(), ']', errc.message());
    }
    catch (const std::exception& e) {
        serverLogger.error(ERROR_WITH_STACKTRACE(e.what()));
    }
    serverManager.removeSocket(socket_ptr);
    co_return;
}

asio::awaitable<void> qls::Network::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { asio::ip::make_address(m_host), m_port });
    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

inline std::string qls::socket2ip(const qls::Socket& s)
{
    auto ep = s.lowest_layer().remote_endpoint();
    return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
}

inline std::string qls::showBinaryData(std::string_view data)
{
    auto isShowableCharacter = [](unsigned char ch) -> bool {
        return 32 <= ch && ch <= 126;
        };

    std::string result;
    for (const auto& i : data) {
        if (isShowableCharacter(static_cast<unsigned char>(i)))
            result += i;
        else {
            std::string hex;
            int locch = static_cast<unsigned char>(i);
            while (locch) {
                if (locch % 16 < 10) {
                    hex += ('0' + (locch % 16));
                    locch /= 16;
                    continue;
                }
                switch (locch % 16) {
                case 10:
                    hex += 'a';
                    break;
                case 11:
                    hex += 'b';
                    break;
                case 12:
                    hex += 'c';
                    break;
                case 13:
                    hex += 'd';
                    break;
                case 14:
                    hex += 'e';
                    break;
                case 15:
                    hex += 'f';
                    break;
                }
                locch /= 16;
            }

            if (hex.empty())
                result += "\\x00";
            else if (hex.size() == 1)
                result += "\\x0" + hex;
            else
                result += "\\x" + hex;
        }
    }

    return result;
}
