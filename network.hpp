#pragma once

#include <asio.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <print>

#include <QuqiCrypto.hpp>

namespace qls
{
	using asio::ip::tcp;
	using asio::awaitable;
	using asio::co_spawn;
	using asio::detached;
	using asio::use_awaitable;
	namespace this_coro = asio::this_coro;

    class Network
    {
    public:
		template<typename Ty>
			requires std::integral<Ty>
		class Package
		{
		public:
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
			{
				m_buffer += data;
			}

			/*
			* @brief 是否可以读取
			* @return true 是, false 否
			*/
			bool canRead() const
			{
				if (m_buffer.size() < sizeof(Ty))
					return false;

				Ty length = 0;
				memcpy_s(&length, sizeof(Ty), m_buffer.c_str(), sizeof(Ty));
				if (length + sizeof(Ty) > m_buffer.size())
					return false;

				return true;
			}

			/*
			* @brief 第一个数据包的长度
			* @return size 第一个数据包的长度
			*/
			size_t firstMsgLength()
			{
				Ty length = 0;
				memcpy_s(&length, sizeof(Ty), m_buffer.c_str(), sizeof(Ty));
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

				std::string result = m_buffer.substr(0, firstMsgLength() + sizeof(Ty));
				m_buffer = m_buffer.substr(firstMsgLength() + sizeof(Ty));

				return result;
			}

			/*
			* @brief 制造数据包
			* @param 二进制数据
			* @return 经过数据包包装的二进制数据
			*/
			static std::string makePackage(std::string_view data)
			{
				Ty lenght = static_cast<Ty>(data.size());
				std::string result;
				result.resize(sizeof(Ty));
				memcpy_s(result.data(), sizeof(Ty), result, sizeof(Ty));
				result += data;

				return result;
			}

		private:
			std::string m_buffer;
		};

		using acceptFunction = std::function<asio::awaitable<void>(tcp::socket&)>;
		using receiveFunction = std::function<asio::awaitable<void>(tcp::socket&, std::string)>;
		using closeFunction = std::function<asio::awaitable<void>(tcp::socket&)>;

		Network() :
			port_(55555),
			thread_num_((12 > int(std::thread::hardware_concurrency())
				? int(std::thread::hardware_concurrency()) : 12))
		{
			threads_ = new std::thread[thread_num_ + 1]{};
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
				std::print("Exception: {}\n", e.what());
			}
		}

	protected:
		awaitable<void> echo(tcp::socket socket)
		{
			auto executor = co_await this_coro::executor;

			bool has_closed = false;
			std::string error_msg;
			try
			{
				co_await acceptFunction_(socket);

				char data[1024];
				Package<int> package;
				for (;;)
				{
					do
					{
						std::size_t n = co_await socket.async_read_some(asio::buffer(data), use_awaitable);
						package.write({ data,n });
					} while (!package.canRead());

					while (package.canRead())
					{
						co_await receiveFunction_(socket, package.read());
					}
				}
			}
			catch (std::exception& e)
			{
				if (!strcmp(e.what(), "End of file"))
				{
					has_closed = true;
				}
				else
				{
					std::print("error: {}\n", e.what());
				}
			}

			if (has_closed)
			{
				co_await closeFunction_(socket);
			}
			co_return;
		}

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
		std::string host_;
		unsigned short port_;
		std::thread*	threads_;
		const int		thread_num_;
		acceptFunction acceptFunction_;
		receiveFunction receiveFunction_;
		closeFunction closeFunction_;
    };
}

