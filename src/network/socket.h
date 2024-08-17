#ifndef SOCKET_H
#define SOCKET_H

#include <asio.hpp>
#include <asio/ssl.hpp>

namespace qls
{
    using Socket = asio::ssl::stream<asio::ip::tcp::socket>;
}

#endif // !SOCKET_H
