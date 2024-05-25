#ifndef ROOM_H
#define ROOM_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <memory>
#include <functional>
#include <stdexcept>

#include "socket.h"

namespace qls
{
    struct BaseRoomImpl;

    /*
    * @brief 基类房间
    */
    class BaseRoom
    {
    public:
        BaseRoom();
        virtual ~BaseRoom() = default;

        virtual bool joinRoom(
            const std::shared_ptr<Socket>& socket_ptr,
            long long user_id);

        virtual bool leaveRoom(long long user_id,
            const std::shared_ptr<Socket>& socket_ptr);

        virtual asio::awaitable<void> sendData(const std::string& data);
        virtual asio::awaitable<void> sendData(const std::string& data, long long user_id);

        virtual void sendData(const std::string& data, std::function<void(std::error_code, size_t)>);
        virtual void sendData(const std::string& data, long long user_id, std::function<void(std::error_code, size_t)>);

    private:
        std::shared_ptr<BaseRoomImpl> m_impl;
    };
}

#endif // !ROOM_H
