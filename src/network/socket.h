#ifndef SOCKET_H
#define SOCKET_H

#include <asio.hpp>
#include <asio/ssl.hpp>

namespace qls
{
    using Socket = asio::ssl::stream<asio::ip::tcp::socket>;

    // class Socket: public asio::ssl::stream<asio::ip::tcp::socket>
    // {
    // public:
    //     Socket(asio::ip::tcp::socket&&, asio::ssl::context&);
    //     ~Socket() = default;

    //     Socket(Socket&&) noexcept;
    //     Socket(const Socket&) = delete;
    // };
}

#endif // !SOCKET_H
