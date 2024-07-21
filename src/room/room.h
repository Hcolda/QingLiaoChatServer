#ifndef ROOM_H
#define ROOM_H

#include <memory>
#include <functional>
#include <stdexcept>
#include <string_view>

#include "user.h"

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

        virtual bool joinRoom(long long user_id, const std::shared_ptr<User>& user_ptr);
        virtual bool leaveRoom(long long user_id);

        virtual void sendData(std::string_view data);
        virtual void sendData(std::string_view data, long long user_id);

    private:
        std::shared_ptr<BaseRoomImpl> m_impl;
    };
}

#endif // !ROOM_H
