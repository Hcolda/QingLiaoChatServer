#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include "room.h"

namespace qls
{
    class GroupRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        struct UserDataStruct
        {
            std::string nickname;
            long long groupLevel;
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

        /*enum class ReturnState
        {
            UNKNOWN_STATE = 0,
            OK_STATE,
            NO_MEMBER_STATE,
            MEMBER_MUTED_STATE
        };*/

        class GroupPermission
        {
        public:
            enum class PermissionType
            {
                Default = 0,
                Operator,
                Administrator
            };

            GroupPermission() = default;
            ~GroupPermission() = default;

            void modifyPermission(const std::string& permissionName, PermissionType type = PermissionType::Default);
            void removePermission(const std::string& permissionName);
            PermissionType getPermissionType(const std::string& permissionName) const;
            
            void modifyUserPermission(long long user_id, PermissionType type = PermissionType::Default);
            void removeUser(long long user_id);
            bool userHasPermission(long long user_id, const std::string& permissionName) const;
            PermissionType getUserPermissionType(long long user_id) const;

        private:
            std::unordered_map<std::string, PermissionType> m_permission_map;
            mutable std::shared_mutex                       m_permission_map_mutex;

            std::unordered_map<long long, PermissionType>   m_user_permission_map;
            mutable std::shared_mutex                       m_user_permission_map_mutex;
        };

        GroupRoom(long long group_id);
        ~GroupRoom() = default;

        void init();
        bool addMember(long long user_id);
        bool removeMember(long long user_id);
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);
        
        asio::awaitable<bool> sendMessage(long long sender_user_id, const std::string& message);
        asio::awaitable<bool> sendTipMessage(long long sender_user_id, const std::string& message);
        asio::awaitable<bool> sendUserTipMessage(long long sender_user_id, const std::string& message, long long receiver_user_id);
       
        bool hasUser(long long user_id) const;
        long long getAdministrator() const;
        void setAdministrator(long long user_id);
        long long getGroupID() const;
        
        bool muteUser(long long executorId, long long user_id, const std::chrono::minutes& mins);
        bool unmuteUser(long long executorId, long long user_id);
        bool kickUser(long long executorId, long long user_id);
        bool addOperator(long long executorId, long long user_id);
        bool removeOperator(long long executorId, long long user_id);

    private:
        const long long                 m_group_id;
        long long                       m_administrator_user_id;
        mutable std::shared_mutex       m_administrator_user_id_mutex;

        GroupPermission                 m_permission;

        std::unordered_map<long long,
            UserDataStruct>             m_user_id_map;
        mutable std::shared_mutex       m_user_id_map_mutex;

        std::unordered_map<long long,
            std::pair<std::chrono::system_clock::time_point,
                std::chrono::minutes>>  m_muted_user_map;
        mutable std::shared_mutex       m_muted_user_map_mutex;

        std::vector<std::pair<std::chrono::system_clock::time_point,
            MessageStruct>>             m_message_queue;
        mutable std::shared_mutex       m_message_queue_mutex;
    };
}

#endif // !GROUP_ROOM_H
