#ifndef SOCKET_H
#define SOCKET_H

#include <asio.hpp>
#include <asio/ssl.hpp>

namespace qls
{
    class Socket : public asio::ssl::stream<asio::ip::tcp::socket>
    {
    public:
        Socket(asio::ip::tcp::socket, asio::ssl::context&);
        virtual ~Socket() = default;
    };
}

#endif // !SOCKET_H
