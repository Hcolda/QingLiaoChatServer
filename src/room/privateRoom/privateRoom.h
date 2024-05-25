#ifndef PRIVATE_ROOM_H
#define PRIVATE_ROOM_H

#include <chrono>
#include <asio.hpp>
#include <string>
#include <vector>
#include <shared_mutex>
#include <unordered_map>

#include "room.h"

namespace qls
{
    /*
    * @brief 私聊房间基类
    */
    class PrivateRoom : public qls::BaseRoom
    {
    public:
        struct MessageStruct
        {
            enum class MessageType
            {
                NOMAL_MESSAGE = 0,
                TIP_MESSAGE
            };

            long long user_id;
            std::string message;
            MessageType type;
        };

        PrivateRoom(long long user_id_1, long long user_id_2, bool is_create);
        PrivateRoom(const PrivateRoom&) = delete;
        PrivateRoom(PrivateRoom&&) = delete;

        ~PrivateRoom() = default;

        asio::awaitable<void> sendMessage(const std::string& message, long long sender_user_id);
        asio::awaitable<void> sendTipMessage(const std::string& message, long long sender_user_id);
        asio::awaitable<void> getMessage(
            const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
            const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to);

        long long getUserID1() const;
        long long getUserID2() const;
        bool hasUser(long long user_id) const;

        void removeThisRoom();
        bool canBeUsed() const;

    private:
        const long long m_user_id_1, m_user_id_2;

        std::atomic<bool>               m_can_be_used;

        std::vector<std::pair<std::chrono::time_point<std::chrono::system_clock,
            std::chrono::milliseconds>,
            MessageStruct>>             m_message_queue;
        mutable std::shared_mutex       m_message_queue_mutex;
    };
}

#endif // !PRIVATE_ROOM_H
