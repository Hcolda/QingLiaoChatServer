#ifndef PRIVATE_ROOM_H
#define PRIVATE_ROOM_H

#include <chrono>
#include <asio.hpp>
#include <string>
#include <vector>
#include <shared_mutex>
#include <unordered_map>
#include <string_view>

#include "userid.hpp"
#include "room.h"

namespace qls
{
    
/*
* @brief 私聊房间基类
*/
class PrivateRoom: protected TextDataRoom
{
public:
    PrivateRoom(UserID user_id_1, UserID user_id_2, bool is_create);
    PrivateRoom(const PrivateRoom&) = delete;
    PrivateRoom(PrivateRoom&&) = delete;

    ~PrivateRoom() noexcept = default;

    void sendMessage(std::string_view message, UserID sender_user_id);
    void sendTipMessage(std::string_view message, UserID sender_user_id);
    void getMessage(
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to);
        
    std::pair<UserID, UserID> getUserID() const;
    bool hasMember(UserID user_id) const;

    void removeThisRoom();
    bool canBeUsed() const;

private:
    const UserID m_user_id_1, m_user_id_2;

    std::atomic<bool>               m_can_be_used;

    std::vector<std::pair<std::chrono::time_point<std::chrono::system_clock,
        std::chrono::milliseconds>,
        MessageStructure>>          m_message_queue;
    mutable std::shared_mutex       m_message_queue_mutex;
};

} // namespace qls

#endif // !PRIVATE_ROOM_H
