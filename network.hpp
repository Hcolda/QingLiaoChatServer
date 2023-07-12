#pragma once

#include <asio.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <QuqiCoro.hpp>
#include <string>

namespace qls
{
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

		void write(std::string_view data)
		{
			m_buffer += data;
		}

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

		Ty firstMsgLength()
		{
			Ty length = 0;
			memcpy_s(&length, sizeof(Ty), m_buffer.c_str(), sizeof(Ty));
			return length;
		}

		std::string read()
		{
			if (!canRead())
				throw std::logic_error("Can't read data");

			std::string result = m_buffer.substr(0, firstMsgLength() + sizeof(Ty));
			m_buffer = m_buffer.substr(firstMsgLength() + sizeof(Ty));

			return result;
		}

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

	using asio::ip::tcp;
	using asio::awaitable;
	using asio::co_spawn;
	using asio::detached;
	using asio::use_awaitable;
	namespace this_coro = asio::this_coro;

    class Network
    {
    public:
		using acceptFunction = std::function<asio::awaitable<void>(tcp::socket&)>;
		using receiveFunction = std::function<asio::awaitable<void>(tcp::socket&, std::string)>;
		using closeFunction = std::function<asio::awaitable<void>(tcp::socket&)>;

		Network() :
			port_(55555)
		{

		}
		~Network()
		{
			thread_executor_.join();
		}

		awaitable<void> echo(tcp::socket socket)
		{
			auto executor = co_await this_coro::executor;

			co_spawn(executor, acceptFunction_(socket), detached);

			try
			{
				char data[1024];
				Package<int> package;
				for (;;)
				{
					do
					{
						std::size_t n = co_await socket.async_read_some(asio::buffer(data), use_awaitable);
						package.write({ data,n });
					} while (!package.canRead());

					co_spawn(executor, receiveFunction_(socket, package.read()), detached);
				}
			}
			catch (std::exception& e)
			{
				if (!strcmp(e.what(), "End of file"))
				{
					co_spawn(executor, closeFunction_(socket), detached);
				}
				else
				{
					std::printf(e.what());
				}
			}
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

		void setFunctions(acceptFunction a, receiveFunction r, closeFunction c)
		{
			acceptFunction_ = std::move(a);
			receiveFunction_ = std::move(r);
			closeFunction_ = std::move(c);
		}

		void run(std::string_view host, unsigned short port)
		{
			host_ = host;
			port_ = port;

			try
			{
				asio::io_context io_context;

				asio::signal_set signals(io_context, SIGINT, SIGTERM);
				signals.async_wait([&](auto, auto) { io_context.stop(); });

				co_spawn(io_context, listener(), detached);

				for (int i = 0;
					i < (12 > static_cast<int>(std::thread::hardware_concurrency())
					? static_cast<int>(std::thread::hardware_concurrency()) : 12);
					i++)
				{
					thread_executor_.post([&]() {
						io_context.run();
						});
				}
			}
			catch (const std::exception& e)
			{
				std::printf("Exception: %s\n", e.what());
			}
		}

	private:
		std::string host_;
		unsigned short port_;
		qcoro::thread_pool_executor thread_executor_;
		acceptFunction acceptFunction_;
		receiveFunction receiveFunction_;
		closeFunction closeFunction_;
    };
}

