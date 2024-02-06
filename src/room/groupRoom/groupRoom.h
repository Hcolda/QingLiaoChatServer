#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include <chrono>
#include <vector>
#include <shared_mutex>
#include <unordered_map>
#include <asio.hpp>

#include "room.h"
#include "groupPermission.h"

namespace qls
{
    class GroupRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        struct UserDataStruct
        {
            std::string nickname;
            long long groupLevel = 1;
        };

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

        GroupRoom(long long group_id, long long administrator, bool is_create);
        GroupRoom(const GroupRoom&) = delete;
        GroupRoom(GroupRoom&&) = delete;
        ~GroupRoom() = default;

        bool addMember(long long user_id);
        bool removeMember(long long user_id);
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);
        
        asio::awaitable<bool> sendMessage(long long sender_user_id, const std::string& message);
        asio::awaitable<bool> sendTipMessage(long long sender_user_id, const std::string& message);
        asio::awaitable<bool> sendUserTipMessage(long long sender_user_id, const std::string& message, long long receiver_user_id);
        asio::awaitable<bool> getMessage(
            const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
            const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to);

        bool hasUser(long long user_id) const;
        std::unordered_map<long long,
            UserDataStruct> getUserList() const;
        std::unordered_map<long long,
            GroupPermission::PermissionType> getUserPermissionList() const;
        long long getAdministrator() const;
        void setAdministrator(long long user_id);
        long long getGroupID() const;
        
        bool muteUser(long long executorId, long long user_id, const std::chrono::minutes& mins);
        bool unmuteUser(long long executorId, long long user_id);
        bool kickUser(long long executorId, long long user_id);
        bool addOperator(long long executorId, long long user_id);
        bool removeOperator(long long executorId, long long user_id);

        void removeThisRoom();
        bool canBeUsed() const;

    private:
        const long long                 m_group_id;
        long long                       m_administrator_user_id;
        mutable std::shared_mutex       m_administrator_user_id_mutex;

        std::atomic<bool>               m_can_be_used;

        GroupPermission                 m_permission;

        std::unordered_map<long long,
            UserDataStruct>             m_user_id_map;
        mutable std::shared_mutex       m_user_id_map_mutex;

        std::unordered_map<long long,
            std::pair<std::chrono::time_point<std::chrono::system_clock,
            std::chrono::milliseconds>,
                std::chrono::minutes>>  m_muted_user_map;
        mutable std::shared_mutex       m_muted_user_map_mutex;

        std::vector<std::pair<std::chrono::time_point<std::chrono::system_clock,
            std::chrono::milliseconds>,
            MessageStruct>>             m_message_queue;
        mutable std::shared_mutex       m_message_queue_mutex;
    };
}

#endif // !GROUP_ROOM_H
