#include "network.h"

#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <future>

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <logger.hpp>
#include <Json.h>

#include "definition.hpp"
#include <package.h>
#include <dataPackage.h>

namespace qls
{
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;
    using namespace std::placeholders;
    using asio::ip::tcp;
    using namespace asio;
    using namespace std::chrono;

    using ssl_socket = asio::ssl::stream<tcp::socket>;

    std::string socket2ip(const asio::ip::tcp::socket& s)
    {
        auto ep = s.remote_endpoint();
        return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
    }

    std::string showBinaryData(const std::string& data)
    {
        auto isShowableCharactor = [](unsigned char ch) -> bool {
            return 32 <= ch && ch <= 126;
            };

        std::string result;

        for (const auto& i : data) {
            if (isShowableCharactor(static_cast<unsigned char>(i)))
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

                //result += "\\x" + (hex.size() == 1 ? "0" + hex : hex);
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

    struct NetworkImpl
    {
#ifdef _DEBUG
        std::string host = "127.0.0.1";
#else
        std::string host = "hcolda.qqof.top";
#endif // DEBUG
        int         port = 55555;

        std::thread                             work_thread;
        asio::io_context                        io_context;
        asio::ssl::context                      ssl_context;
        std::shared_ptr<ssl_socket>             socket_ptr;
        std::atomic<bool>                       is_running;
        std::atomic<bool>                       is_receiving;
        std::atomic<bool>                       has_stopped;
        std::mutex                              mutex;
        std::condition_variable                 condition_variable;
        Package                                 package;
        asio::ip::tcp::resolver::results_type   endpoints;
        std::string                             input_buffer;

        asio::steady_timer                      deadline_timer;
        asio::steady_timer                      heartbeat_timer;

        std::unordered_map<std::string,
            ReceiveStdStringFunction>   revceiveStdStringFunction_map;
        std::shared_mutex               revceiveStdStringFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      connectedCallbackFunction_map;
        std::shared_mutex               connectedCallbackFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      disconnectedCallbackFunction_map;
        std::shared_mutex               disconnectedCallbackFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void(std::error_code)>>
                                        connectedErrorCallbackFunction_map;
        std::shared_mutex               connectedErrorCallbackFunction_map_mutex;

        std::mt19937_64                 requestID_mt;
        std::mutex                      requestID_mt_mutex;
        std::unordered_set<long long>   requestID_set;
        std::shared_mutex               requestID_set_mutex;
        std::unordered_map<long long,
            std::function<void(std::shared_ptr<DataPackage>)>>
                                        requestID2Function_map;
        std::shared_mutex               requestID2Function_map_mutex;

        NetworkImpl() :
            deadline_timer(io_context),
            heartbeat_timer(io_context),
            ssl_context(asio::ssl::context::tlsv13_client),
            requestID_mt(std::random_device{}())
        {
            input_buffer.resize(8192);

            // ssl 配置
            ssl_context.set_default_verify_paths();

            // 设置ssl参数
            ssl_context.set_options(
                asio::ssl::context::default_workarounds
                | asio::ssl::context::no_sslv2
                | asio::ssl::context::no_sslv3
                | asio::ssl::context::no_tlsv1
                | asio::ssl::context::no_tlsv1_1
                | asio::ssl::context::no_tlsv1_2
                | asio::ssl::context::single_dh_use
            );

            // 设置是否验证cert
#ifdef _DEBUG
            ssl_context.set_verify_mode(asio::ssl::verify_none);
#else
            ssl_context.set_verify_mode(asio::ssl::verify_peer);
#endif // _DEBUG

            ssl_context.set_verify_callback(
                std::bind(&NetworkImpl::verify_certificate, this, _1, _2));
            // ssl_context.set_verify_callback(ssl::rfc2818_verification("host.name"));

            socket_ptr = std::make_shared<ssl_socket>(io_context, ssl_context);
        }

        ~NetworkImpl() = default;

        bool verify_certificate(bool preverified,
            asio::ssl::verify_context& ctx)
        {
            char subject_name[256];
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            std::cout << "Verifying " << subject_name << "\n";

            return preverified;
        }
    };

    Network::Network() :
        m_network_impl(std::make_shared<NetworkImpl>())
    {
        m_network_impl->is_running = true;
        m_network_impl->is_receiving = false;
        m_network_impl->has_stopped = false;
        
        m_network_impl->work_thread = std::thread([&]() {
            while (m_network_impl->is_running) {
                std::unique_lock<std::mutex> lock(m_network_impl->mutex);
                m_network_impl->condition_variable.wait(lock,
                    [&]() { return !m_network_impl->is_running || !m_network_impl->endpoints.empty(); });
                if (!m_network_impl->is_running)
                    return;
                try {
                    lock.unlock();
                    start_connect();
                    m_network_impl->io_context.run();
                } catch (...) {}
            }
        });
    }

    Network::~Network() noexcept
    {
        if (m_network_impl->has_stopped)
            return;
        m_network_impl->has_stopped = true;
        m_network_impl->is_running = false;
        if (m_network_impl->is_receiving) {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->shutdown(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
        }
        m_network_impl->io_context.stop();
        m_network_impl->condition_variable.notify_all();
        if (m_network_impl->work_thread.joinable())
            m_network_impl->work_thread.join();
    }

    void Network::connect()
    {
        asio::io_service io_service;
        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::query resolver_query(m_network_impl->host,
            std::to_string(m_network_impl->port));
        std::error_code ec;
        asio::ip::tcp::resolver::iterator it =
            resolver.resolve(resolver_query, ec);
        asio::ip::tcp::resolver::iterator it_end;

        if (it == it_end)
            throw std::runtime_error("there is not a service match the host");
        {
            m_network_impl->is_receiving = false;
            std::unique_lock<std::mutex> lock(m_network_impl->mutex);
            m_network_impl->endpoints = resolver.resolve(resolver_query, ec);
        }
        m_network_impl->condition_variable.notify_all();
    }

    void Network::disconnect()
    {
        m_network_impl->is_receiving = false;
        {
            std::unique_lock<std::mutex> lock(m_network_impl->mutex);
            std::error_code ignored_error;
            m_network_impl->socket_ptr->shutdown(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
        }
        m_network_impl->condition_variable.notify_all();
    }

    void Network::stop()
    {
        if (m_network_impl->has_stopped)
            return;
        m_network_impl->has_stopped = true;
        m_network_impl->is_running = false;
        if (m_network_impl->is_receiving) {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->shutdown(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
        }
        m_network_impl->io_context.stop();
        m_network_impl->condition_variable.notify_all();
    }

    void Network::send_data(const std::string& data)
    {
        if (!m_network_impl->is_receiving)
            throw std::runtime_error("Socket is not able to use");
        auto wrapper = std::make_shared<StringWrapper>(data);
        asio::async_write(*(m_network_impl->socket_ptr),
            asio::buffer(wrapper->data), std::bind(&Network::handle_write, this, _1, _2, wrapper));
    }

    std::future<std::shared_ptr<DataPackage>> Network::send_data_with_result_n_option(const std::string& data,
        const std::function<void(std::shared_ptr<DataPackage>&)>& option_function)
    {
        if (!m_network_impl->is_receiving)
            throw std::runtime_error("Socket is not able to use");
        std::shared_ptr<std::promise<std::shared_ptr<DataPackage>>> future_result = 
            std::make_shared<std::promise<std::shared_ptr<DataPackage>>>();
        send_data_with_option(data, option_function,
            [future_result](std::shared_ptr<DataPackage> pack) {
                future_result->set_value(std::move(pack));
            });
        return future_result->get_future();
    }

    long long Network::send_data_with_option(const std::string& origin_data,
        const std::function<void(std::shared_ptr<DataPackage>&)>& option_function,
        const std::function<void(std::shared_ptr<DataPackage>)>& callback_function)
    {
        if (!m_network_impl->is_receiving)
            throw std::runtime_error("Socket is not able to use");
        if (!option_function || !callback_function)
            throw std::runtime_error("Functions is null");
        auto pack = DataPackage::makePackage(origin_data);
        option_function(pack);
        long long requestId = 0;
        {
            std::unique_lock<std::mutex> mt_lock(m_network_impl->requestID_mt_mutex, std::defer_lock);
            std::unique_lock<std::shared_mutex> set_lock(m_network_impl->requestID_set_mutex, std::defer_lock),
                map_lock(m_network_impl->requestID2Function_map_mutex, std::defer_lock);
            std::lock(mt_lock, set_lock, map_lock);
            do {
                requestId = m_network_impl->requestID_mt();
            } while (m_network_impl->requestID_set.find(requestId) != m_network_impl->requestID_set.cend());
            m_network_impl->requestID_set.insert(requestId);
        }
        pack->requestID = requestId;
        m_network_impl->requestID2Function_map[requestId] = callback_function;
        send_data(pack->packageToString());
        return requestId;
    }

    bool Network::add_received_stdstring_callback(const std::string& name, ReceiveStdStringFunction func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->revceiveStdStringFunction_map_mutex);
        auto iter = m_network_impl->revceiveStdStringFunction_map.find(name);
        if (iter != m_network_impl->revceiveStdStringFunction_map.end())
            return false;
        m_network_impl->revceiveStdStringFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_received_stdstring_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->revceiveStdStringFunction_map_mutex);
        auto iter = m_network_impl->revceiveStdStringFunction_map.find(name);
        if (iter == m_network_impl->revceiveStdStringFunction_map.end())
            return false;
        m_network_impl->revceiveStdStringFunction_map.erase(iter);
        return true;
    }

    bool Network::add_connected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedCallbackFunction_map_mutex);
        auto iter = m_network_impl->connectedCallbackFunction_map.find(name);
        if (iter != m_network_impl->connectedCallbackFunction_map.end())
            return false;
        m_network_impl->connectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_connected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedCallbackFunction_map_mutex);
        auto iter = m_network_impl->connectedCallbackFunction_map.find(name);
        if (iter == m_network_impl->connectedCallbackFunction_map.end())
            return false;
        m_network_impl->connectedCallbackFunction_map.erase(iter);
        return true;
    }

    bool Network::add_disconnected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->disconnectedCallbackFunction_map_mutex);
        auto iter = m_network_impl->disconnectedCallbackFunction_map.find(name);
        if (iter != m_network_impl->disconnectedCallbackFunction_map.end())
            return false;
        m_network_impl->disconnectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_disconnected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->disconnectedCallbackFunction_map_mutex);
        auto iter = m_network_impl->disconnectedCallbackFunction_map.find(name);
        if (iter == m_network_impl->disconnectedCallbackFunction_map.end())
            return false;
        m_network_impl->disconnectedCallbackFunction_map.erase(iter);
        return true;
    }

    bool Network::add_connected_error_callback(const std::string& name, std::function<void(std::error_code)> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedErrorCallbackFunction_map_mutex);
        auto iter = m_network_impl->connectedErrorCallbackFunction_map.find(name);
        if (iter != m_network_impl->connectedErrorCallbackFunction_map.end())
            return false;
        m_network_impl->connectedErrorCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_connected_error_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedErrorCallbackFunction_map_mutex);
        auto iter = m_network_impl->connectedErrorCallbackFunction_map.find(name);
        if (iter == m_network_impl->connectedErrorCallbackFunction_map.end())
            return false;
        m_network_impl->connectedErrorCallbackFunction_map.erase(iter);
        return true;
    }

    void Network::call_connected()
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->connectedCallbackFunction_map_mutex);
        for (const auto& [_, func]: m_network_impl->connectedCallbackFunction_map) {
            func();
        }
    }

    void Network::call_disconnect()
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->disconnectedCallbackFunction_map_mutex);
        for (const auto& [_, func] : m_network_impl->disconnectedCallbackFunction_map) {
            func();
        }
    }

    void Network::call_connected_error(const std::error_code& error)
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->connectedErrorCallbackFunction_map_mutex);
        for (const auto& [_, func] : m_network_impl->connectedErrorCallbackFunction_map) {
            func(error);
        }
    }

    void Network::call_received_stdstring(std::string data)
    {
        try {
            auto pack = DataPackage::stringToPackage(data);
            long long requestID = pack->requestID;
            if (pack->requestID != 0) {
                std::unique_lock<std::shared_mutex> map_lock(m_network_impl->requestID2Function_map_mutex,
                    std::defer_lock),
                    set_lock(m_network_impl->requestID_set_mutex, std::defer_lock);
                bool need_moving_data = false;
                std::lock(map_lock, set_lock);
                {
                    auto iter = m_network_impl->requestID_set.find(requestID);
                    if (iter != m_network_impl->requestID_set.cend()) {
                        need_moving_data = true;
                        m_network_impl->requestID_set.erase(iter);
                    }
                }
                if (need_moving_data) {
                    auto iter = m_network_impl->requestID2Function_map.find(requestID);
                    if (iter != m_network_impl->requestID2Function_map.cend())
                        iter->second(std::move(pack));
                    m_network_impl->requestID2Function_map.erase(iter);
                    return;
                }
            }
        } catch (...) {
            return;
        }
        std::shared_lock<std::shared_mutex> lock(m_network_impl->revceiveStdStringFunction_map_mutex);
        for (const auto& [_, func] : m_network_impl->revceiveStdStringFunction_map) {
            func(data);
        }
    }

    void Network::start_connect()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running)
            return;
        m_network_impl->deadline_timer.expires_after(std::chrono::seconds(60));
        asio::async_connect(m_network_impl->socket_ptr->lowest_layer(),
            m_network_impl->endpoints.begin(),
            m_network_impl->endpoints.end(),
            std::bind(&Network::handle_connect, this, _1));
    }

    void Network::handle_connect(const std::error_code& error)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running)
            return;
        if (error) {
            std::error_code ec;
            m_network_impl->socket_ptr->lowest_layer().close(ec);
            lock.unlock();
            call_connected_error(error);
            std::this_thread::sleep_for(10s);
            start_connect();
        } else {
            lock.unlock();
            async_handshake();
        }
    }

    void Network::async_handshake()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running)
            return;
        m_network_impl->deadline_timer.expires_after(std::chrono::seconds(10));
        m_network_impl->socket_ptr->async_handshake(asio::ssl::stream_base::client,
            std::bind(&Network::handle_handshake, this, _1));
    }

    void Network::handle_handshake(const std::error_code& error)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running)
            return;
        if (!error) {
            m_network_impl->is_receiving = true;
            lock.unlock();
            send_data(DataPackage::makePackage("test")->packageToString());
            async_read();
            heart_beat_write();
            call_connected();
        } else {
            std::error_code ec;
            m_network_impl->socket_ptr->shutdown(ec);
            std::cout << error.message() << '\n';
            call_connected_error(error);
        }
    }

    void Network::async_read()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;
        m_network_impl->deadline_timer.expires_after(std::chrono::seconds(30));
        m_network_impl->socket_ptr->async_read_some(asio::buffer(m_network_impl->input_buffer),
            std::bind(&Network::handle_read, this, _1, _2));
    }

    void Network::handle_read(const std::error_code& error, std::size_t n)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;
        if (!error) {
            if (!n) {
                async_read();
                return;
            }
            m_network_impl->package.write({ m_network_impl->input_buffer.begin(),
                m_network_impl->input_buffer.begin() + n });
            m_network_impl->input_buffer.clear();
            m_network_impl->input_buffer.resize(8192);

            if (m_network_impl->package.canRead())
                call_received_stdstring(std::move(
                    m_network_impl->package.read()));

            lock.unlock();
            async_read();
        } else {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->shutdown(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
        }
    }

    void Network::heart_beat_write()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;
        auto pack = DataPackage::makePackage("heartbeat");
        pack->type = DataPackage::HeartBeat;
        auto wrapper = std::make_shared<StringWrapper>(pack->packageToString());
        asio::async_write(*(m_network_impl->socket_ptr), asio::buffer(wrapper->data),
            std::bind(&Network::handle_heart_beat_write, this, _1, wrapper));
    }

    void Network::handle_heart_beat_write(const std::error_code& error, std::shared_ptr<StringWrapper>)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;
        if (!error) {
            m_network_impl->heartbeat_timer.expires_after(std::chrono::seconds(10));
            m_network_impl->heartbeat_timer.async_wait(std::bind(&Network::heart_beat_write, this));
        } else {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->shutdown(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
            call_disconnect();
        }
    }

    void Network::check_deadline()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;
        if (m_network_impl->deadline_timer.expiry() <= asio::steady_timer::clock_type::now()) {
            std::error_code ec;
            m_network_impl->socket_ptr->shutdown(ec);
            m_network_impl->is_receiving = false;

            m_network_impl->deadline_timer.expires_at(asio::steady_timer::time_point::max());
            call_disconnect();

            lock.unlock();
            start_connect();
        }

        m_network_impl->deadline_timer.async_wait(std::bind(&Network::check_deadline, this));
    }

    void Network::handle_write(const std::error_code& error,
        std::size_t n,
        std::shared_ptr<StringWrapper>)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;
        if (error) {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->shutdown(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
            call_disconnect();
        }
    }
}
