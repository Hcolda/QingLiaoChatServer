#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <asio.hpp>

#include <dataPackage.h>

namespace qls
{
    using ReceiveStdStringFunction = std::function<void(std::string)>;

    inline std::string socket2ip(const asio::ip::tcp::socket& s);
    inline std::string showBinaryData(const std::string& data);

    struct NetworkImpl;

    class BaseNetwork
    {
    public:
        BaseNetwork() = default;
        virtual ~BaseNetwork() = default;

        // 禁止复制和移动
        BaseNetwork(const BaseNetwork&) = delete;
        BaseNetwork(BaseNetwork&&) = delete;
        BaseNetwork& operator=(const BaseNetwork&) = delete;
        BaseNetwork& operator=(BaseNetwork&&) = delete;

        virtual void connect() {}
        virtual void disconnect() {}
        virtual void stop() {}

        virtual void send_data(const std::string&) {}

        virtual std::shared_ptr<DataPackage> send_data_with_result_n_option(const std::string& origin_data,
            const std::function<void(std::shared_ptr<DataPackage>&)>& option_function) { return std::shared_ptr<DataPackage>(); }
        // return a request id
        virtual long long send_data_with_option(const std::string& origin_data,
            const std::function<void(std::shared_ptr<DataPackage>&)>& option_function,
            const std::function<void(std::shared_ptr<DataPackage>)>& callback_function) { return 0ll; }

        virtual bool add_received_stdstring_callback(const std::string&,
             ReceiveStdStringFunction) { return false; }

        virtual bool remove_received_stdstring_callback(const std::string&) { return false; }

        virtual bool add_connected_callback(const std::string&,
            std::function<void()>) { return false; }

        virtual bool remove_connected_callback(const std::string&) { return false; }

        virtual bool add_disconnected_callback(const std::string&,
            std::function<void()>) { return false; }

        virtual bool remove_disconnected_callback(const std::string&) { return false; }

        virtual bool add_connected_error_callback(const std::string&,
            std::function<void(std::error_code)>) { return false; }

        virtual bool remove_connected_error_callback(const std::string&) { return false; }
    };

    struct NetworkImpl;

    struct StringWrapper
    {
        std::string data;
    };

    class Network final: public BaseNetwork
    {
    public:
        Network();
        ~Network();

        void connect();
        void disconnect();
        void stop();

        void send_data(const std::string& data);

        virtual std::shared_ptr<DataPackage> send_data_with_result_n_option(const std::string& origin_data,
            const std::function<void(std::shared_ptr<DataPackage>&)>& option_function);
        virtual long long send_data_with_option(const std::string& origin_data,
            const std::function<void(std::shared_ptr<DataPackage>&)>& option_function,
            const std::function<void(std::shared_ptr<DataPackage>)>& callback_function);

        virtual bool add_received_stdstring_callback(const std::string&, ReceiveStdStringFunction);
        virtual bool remove_received_stdstring_callback(const std::string&);

        virtual bool add_connected_callback(const std::string&, std::function<void()>);
        virtual bool remove_connected_callback(const std::string&);
        virtual bool add_disconnected_callback(const std::string&, std::function<void()>);
        virtual bool remove_disconnected_callback(const std::string&);
        virtual bool add_connected_error_callback(const std::string&, std::function<void(std::error_code)>);
        virtual bool remove_connected_error_callback(const std::string&);

    protected:
        void call_connected();
        void call_disconnect();
        void call_connected_error(const std::error_code& = std::make_error_code(std::errc::not_connected));
        void call_received_stdstring(std::string);

        void start_connect();
        void handle_connect(const std::error_code& error);
        void async_handshake();
        void handle_handshake(const std::error_code&);
        void async_read();
        void handle_read(const std::error_code& error, std::size_t n);
        void heart_beat_write();
        void handle_heart_beat_write(const std::error_code& error, std::shared_ptr<StringWrapper>);
        void check_deadline();
        void handle_write(const std::error_code& error, std::size_t n, std::shared_ptr<StringWrapper>);

    private:
        std::shared_ptr<NetworkImpl> m_network_impl;
    };
}

#endif // !NETWORK_HPP
