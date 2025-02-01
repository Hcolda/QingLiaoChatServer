#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <asio.hpp>
#include "Socket.h"

namespace qls
{

struct Connection
{
    Socket socket;
    asio::strand<asio::any_io_executor> strand;

    Connection(asio::ip::tcp::socket s, asio::ssl::context& context):
        strand(asio::make_strand(s.get_executor())),
        socket(std::move(s), context) {}

    ~Connection()
    {
        std::error_code ec;
        socket.shutdown(ec);
        socket.lowest_layer().close(ec);
    }
};

} // namespace qls


#endif // !CONNECTION_HPP
