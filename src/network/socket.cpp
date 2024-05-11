#include "socket.h"

qls::Socket::Socket(asio::ip::tcp::socket s, asio::ssl::context& c) :
    stream(std::move(s), c)
{
}
