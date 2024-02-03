#ifndef PRIVATE_ROOM_H
#define PRIVATE_ROOM_H

#include "room.h"

namespace qls
{
    /*
    * @brief 私聊房间基类
    */
    class PrivateRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        PrivateRoom(long long user_id_1, long long user_id_2, bool is_create);
        PrivateRoom(const PrivateRoom&) = delete;
        PrivateRoom(PrivateRoom&&) = delete;

        ~PrivateRoom() = default;

        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        asio::awaitable<bool> sendMessage(const std::string& message, long long sender_user_id);
        asio::awaitable<bool> sendTipMessage(const std::string& message);

        long long getUserID1() const;
        long long getUserID2() const;
        bool hasUser(long long user_id) const;

    private:
        const long long m_user_id_1, m_user_id_2;

        std::priority_queue<std::pair<long long, std::string>,
            std::vector<std::pair<long long, std::string>>,
            std::greater<std::pair<long long, std::string>>>        m_message_queue;
        std::shared_mutex                                           m_message_queue_mutex;
    };
}

#endif // !PRIVATE_ROOM_H
