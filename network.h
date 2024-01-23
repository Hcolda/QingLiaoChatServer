#pragma once

#include <asio.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

#include <QuqiCrypto.hpp>

#include "definition.hpp"

namespace qls
{
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;

    /*
    * @brief 读取socket地址到string
    * @param socket
    * @return string socket的地址
    */
    inline std::string socket2ip(const asio::ip::tcp::socket& s)
    {
        auto ep = s.remote_endpoint();
        return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
    }

    /*
    * @brief 判断本地序是否为大端序
    * @return true 为大端序 | false 为小端序
    */
    constexpr inline bool isBigEndianness()
    {
        union u_data
        {
            unsigned char   a;
            unsigned int    b;
        } data;

        data.b = 0x12345678;

        return data.a == 0x12;
    }

    /*
    * @brief 端序转换
    * @param value 数据 (整数型)
    * @return 转换端序后的数据
    */
    template<typename T>
        requires std::integral<T>
    constexpr inline T swapEndianness(T value) {
        T result = 0;
        for (size_t i = 0; i < sizeof(value); ++i) {
            result = (result << 8) | ((value >> (8 * i)) & 0xFF);
        }
        return result;
    }

    /*
    * @brief 本地序与网络序互转
    * @param value 数据 (整数型)
    * @return 转换端序后的数据
    */
    template<typename T>
        requires std::integral<T>
    constexpr inline T swapNetworkEndianness(T value)
    {
        if (!isBigEndianness())
            return swapEndianness(value);
        else
            return value;
    }

    class Network
    {
    public:
        /*
        * @brief 数据包处理类
        */
        class Package
        {
        public:
            /*
            * @brief 数据包
            */
            class DataPackage
            {
#pragma pack(1)
            private:
                int                 length = 0;
            public:
                long long           requestID = 0;
                int                 type = 0;
                int                 sequence = -1;
            private:
                unsigned long long  verifyCode = 0;
                char                data[2]{ 0 };
#pragma pack()

            public:

                /*
                * @brief 制作数据包
                * @param data 数据包中需要存的二进制数据
                * @return 带自动回收的数据包
                */
                static std::shared_ptr<DataPackage> makePackage(std::string_view data)
                {
                    static std::hash<std::string_view> hash;
                    std::shared_ptr<DataPackage> package((DataPackage*)new char[sizeof(DataPackage) + data.size()] { 0 });
                    package->length = int(sizeof(DataPackage) + data.size());
                    std::memcpy(package->data, data.data(), data.size());
                    package->verifyCode = hash(data);
                    return package;
                }

                /*
                * @brief 从string中加载数据包
                * @param data 数据包中需要存的二进制数据
                * @return 带自动回收的数据包
                */
                static std::shared_ptr<DataPackage> stringToPackage(const std::string& data)
                {
                    static std::hash<std::string_view> hash;
                    // 数据包过小
                    if (data.size() < sizeof(DataPackage)) throw std::logic_error("data is too small!");

                    // 数据包length
                    int size = 0;
                    std::memcpy(&size, data.c_str(), sizeof(int));
                    size = swapNetworkEndianness(size);

                    // 数据包length与实际大小不符、length小于数据包默认大小、length非常大、数据包结尾不为2 * '\0' 报错处理
                    if (size != data.size() || size < sizeof(DataPackage)) throw std::logic_error("data is invalid!");
                    else if (size > INT32_MAX / 2) throw std::logic_error("data is too large!");
                    else if (data[size_t(size - 1)] || data[size_t(size - 2)]) throw std::logic_error("data is invalid");

                    std::shared_ptr<DataPackage> package((DataPackage*)new char[size] { 0 });
                    std::memcpy(package.get(), data.c_str(), size);

                    // 端序转换
                    package->length = swapNetworkEndianness(package->length);
                    package->requestID = swapNetworkEndianness(package->requestID);
                    package->type = swapNetworkEndianness(package->type);
                    package->sequence = swapNetworkEndianness(package->sequence);
                    package->verifyCode = swapNetworkEndianness(package->verifyCode);

                    size_t gethash = hash(getData(package));
                    if (gethash != package->verifyCode) throw std::logic_error(std::format("hash is different, local hash: {}, pack hash: {}",
                        gethash, package->verifyCode));

                    return package;
                }

                /*
                * @brief 将数据包转换为二进制格式数据包
                * @param dp DataPackage
                * @return 二进制格式数据包
                */
                static std::string packageToString(const std::shared_ptr<DataPackage>& dp)
                {
                    if (dp.get() == nullptr) throw std::logic_error("datapackage is nullptr");

                    size_t localLength = dp->length;

                    // 端序转换
                    dp->length = swapNetworkEndianness(dp->length);
                    dp->requestID = swapNetworkEndianness(dp->requestID);
                    dp->type = swapNetworkEndianness(dp->type);
                    dp->sequence = swapNetworkEndianness(dp->sequence);
                    dp->verifyCode = swapNetworkEndianness(dp->verifyCode);

                    std::string data;
                    data.resize(localLength);
                    std::memcpy(data.data(), dp.get(), localLength);
                    return data;
                }

                /*
                * @brief 获取数据包大小
                * @return size 数据包大小
                */
                static size_t getPackageSize(const std::shared_ptr<DataPackage>& dp)
                {
                    if (dp.get() == nullptr) throw std::logic_error("datapackage is nullptr");

                    int size = 0;
                    std::memcpy(&size, &(dp->length), sizeof(int));
                    return size_t(size);
                }

                /*
                * @brief 获取包中二进制数据大小
                * @return size 二进制数据大小
                */
                static size_t getDataSize(const std::shared_ptr<DataPackage>& dp)
                {
                    if (dp.get() == nullptr) throw std::logic_error("datapackage is nullptr");

                    int size = 0;
                    std::memcpy(&size, &(dp->length), sizeof(int));
                    return size_t(size) - sizeof(DataPackage);
                }

                /*
                * @brief 获取包中二进制数据
                * @return string 二进制数据
                */
                static std::string getData(const std::shared_ptr<DataPackage>& dp)
                {
                    if (dp.get() == nullptr) throw std::logic_error("datapackage is nullptr");

                    std::string data;
                    size_t size = getDataSize(dp);
                    data.resize(size);
                    std::memcpy(data.data(), dp->data, size);
                    return data;
                }
            };

            Package() = default;
            ~Package() = default;

            Package(const Package&) = delete;
            Package(Package&&) = delete;

            Package& operator =(const Package&) = delete;
            Package& operator =(Package&&) = delete;

            /*
            * @brief 写入数据到类中
            * @param data 二进制数据
            */
            void write(std::string_view data)
            {;
                m_buffer += data;
            }

            /*
            * @brief 是否可以读取
            * @return true 是, false 否
            */
            bool canRead() const
            {
                if (m_buffer.size() < sizeof(int))
                    return false;

                int length = 0;
                std::memcpy(&length, m_buffer.c_str(), sizeof(int));
                length = swapNetworkEndianness(length);
                if (length > m_buffer.size())
                    return false;

                return true;
            }

            /*
            * @brief 第一个数据包的长度
            * @return size 第一个数据包的长度
            */
            size_t firstMsgLength() const
            {
                if (m_buffer.size() < sizeof(int))
                    return 0;

                int length = 0;
                std::memcpy(&length, m_buffer.c_str(), sizeof(int));
                length = swapNetworkEndianness(length);
                return size_t(length);
            }

            /*
            * @brief 读取数据包
            * @return 返回数据包
            */
            std::string read()
            {
                if (!canRead())
                    throw std::logic_error("Can't read data");
                else if (!firstMsgLength())
                    throw std::logic_error("length is empty");

                std::string result = m_buffer.substr(0, firstMsgLength());
                m_buffer = m_buffer.substr(firstMsgLength());

                return result;
            }

            /*
            * @brief 读取类中buffer数据
            * @return string buffer
            */
            const std::string& readBuffer() const
            {
                return m_buffer;
            }

            /*
            * @brief 设置buffer
            * @param buffer
            */
            void setBuffer(const std::string& b)
            {
                m_buffer = b;
            }

            /*
            * @brief 制造数据包
            * @param 二进制数据
            * @return 经过数据包包装的二进制数据
            */
            static std::string makePackage(std::string_view data)
            {
                int lenght = static_cast<int>(data.size());
                std::string result;
                result.resize(sizeof(int));
                std::memcpy(result.data(), result.data(), sizeof(int));
                result += data;

                return result;
            }

        private:
            std::string m_buffer;
        };

        struct SocketDataStructure
        {
            // 用于接收数据包
            Network::Package package;
            // 用于接收密钥
            std::string uuid;

            // 加密等级 1rsa 2aes 0无
            std::atomic<int> has_encrypt = 0;
            std::string AESKey;
            std::string AESiv;
        };

        using acceptFunction = std::function<asio::awaitable<void>(tcp::socket&)>;
        using receiveFunction = std::function<asio::awaitable<void>(tcp::socket&, std::string, std::shared_ptr<Package::DataPackage>)>;
        using closeFunction = std::function<asio::awaitable<void>(tcp::socket&)>;

        Network() :
            port_(55555),
            thread_num_((12 > int(std::thread::hardware_concurrency())
                ? int(std::thread::hardware_concurrency()) : 12))
        {
            threads_ = new std::thread[thread_num_ + 1]{};

            acceptFunction_ = [](tcp::socket&) -> asio::awaitable<void> {co_return;};
            receiveFunction_ = [](tcp::socket&,
                std::string, std::shared_ptr<Package::DataPackage>
                ) -> asio::awaitable<void> {co_return;};
            closeFunction_ = [](tcp::socket&) -> asio::awaitable<void> {co_return;};
        }

        ~Network()
        {
            for (int i = 0; i < thread_num_; i++)
            {
                if (threads_[i].joinable())
                    threads_[i].join();
            }

            delete[] threads_;
        }

        /*
        * @brief 设置函数
        * @param 有新连接时处理函数
        * @param 有数据接收时处理函数
        * @param 有连接主动关闭时的处理函数
        */
        void setFunctions(acceptFunction a, receiveFunction r, closeFunction c)
        {
            acceptFunction_ = std::move(a);
            receiveFunction_ = std::move(r);
            closeFunction_ = std::move(c);
        }

        /*
        * @brief 运行network
        * @param host 主机地址
        * @param port 端口
        */
        void run(std::string_view host, unsigned short port)
        {
            host_ = host;
            port_ = port;

            try
            {
                asio::io_context io_context;

                asio::signal_set signals(io_context, SIGINT, SIGTERM);
                signals.async_wait([&](auto, auto) { io_context.stop(); });

                for (int i = 0; i < thread_num_; i++)
                {
                    threads_[i] = std::thread([&]() {
                        co_spawn(io_context, listener(), detached);
                        io_context.run(); 
                        });
                }

                for (int i = 0; i < thread_num_; i++)
                {
                    if (threads_[i].joinable())
                        threads_[i].join();
                }
            }
            catch (const std::exception& e)
            {
                std::printf("Exception: %s\n", e.what());
            }
        }

    protected:
        awaitable<void> echo(tcp::socket socket);

        awaitable<void> listener()
        {
            auto executor = co_await this_coro::executor;
            tcp::acceptor acceptor(executor, { asio::ip::address::from_string(host_), port_ });
            for (;;)
            {
                tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
                co_spawn(executor, echo(std::move(socket)), detached);
            }
        }

    private:
        std::string     host_;
        unsigned short  port_;
        std::thread*    threads_;
        const int       thread_num_;
        acceptFunction  acceptFunction_;
        receiveFunction receiveFunction_;
        closeFunction   closeFunction_;
    };
}

