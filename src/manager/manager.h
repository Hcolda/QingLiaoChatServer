#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "SQLProcess.hpp"
#include "definition.hpp"
#include "privateRoom.h"
#include "groupRoom.h"
#include "user.h"
#include "friendRoomVerification.h"
#include "groupRoomVerification.h"

namespace qls
{
    class Manager
    {
    public:
        Manager() = default;
        Manager(const Manager&) = delete;
        Manager(Manager&) = delete;
        ~Manager();

        void init();

        void addUserSocket2GlobalRoom(long long user_id, const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);
        std::shared_ptr<BaseRoom> getGlobalRoom() const;

        long long addPrivateRoom(long long user1_id, long long user2_id);
        long long getPrivateRoomId(long long user1_id, long long user2_id) const;
        bool hasPrivateRoom(long long private_room_id) const;
        std::shared_ptr<qls::PrivateRoom> getPrivateRoom(long long private_room_id) const;
        void removePrivateRoom(long long private_room_id);

        long long addGroupRoom(long long opreator_user_id);
        bool hasGroupRoom(long long group_room_id) const;
        std::shared_ptr<qls::GroupRoom> getGroupRoom(long long group_room_id) const;
        void removeGroupRoom(long long group_room_id);

        std::shared_ptr<qls::User> addNewUser();
        bool hasUser(long long user_id) const;
        std::shared_ptr<qls::User> getUser(long long user_id) const;

        void addFriendRoomVerification(long long user_id_1, long long user_id_2);
        bool hasFriendRoomVerification(long long user_id_1, long long user_id_2) const;
        bool setFriendVerified(long long user_id_1, long long user_id_2, long long user_id, bool is_verified);
        void removeFriendRoomVerification(long long user_id_1, long long user_id_2);

        void addGroupRoomVerification(long long group_id, long long user_id);
        bool hasGroupRoomVerification(long long group_id, long long user_id) const;
        bool setGroupRoomGroupVerified(long long group_id, long long user_id, bool is_verified);
        bool setGroupRoomUserVerified(long long group_id, long long user_id, bool is_verified);
        void removeGroupRoomVerification(long long group_id, long long user_id);

        quqisql::SQLDBProcess& getServerSqlProcess();

    private:
        struct PrivateRoomIDStruct
        {
            long long user_id_1;
            long long user_id_2;

            friend bool operator ==(const PrivateRoomIDStruct& a,
                const PrivateRoomIDStruct& b)
            {
                return (a.user_id_1 == b.user_id_1 && a.user_id_2 == b.user_id_2) ||
                    (a.user_id_2 == b.user_id_1 && a.user_id_1 == b.user_id_2);
            }

            friend bool operator !=(const PrivateRoomIDStruct& a,
                const PrivateRoomIDStruct& b)
            {
                return !(a == b);
            }
        };

        class PrivateRoomIDStructHasher
        {
        public:
            PrivateRoomIDStructHasher() = default;
            ~PrivateRoomIDStructHasher() = default;

            size_t operator()(const PrivateRoomIDStruct& s) const
            {
                std::hash<long long> hasher;
                return hasher(s.user_id_1) * hasher(s.user_id_2);
            }
        };

        struct GroupVerificationStruct
        {
            long long group_id;
            long long user_id;

            friend bool operator ==(const GroupVerificationStruct& a,
                const GroupVerificationStruct& b)
            {
                return a.group_id == b.group_id && a.user_id == b.user_id;
            }

            friend bool operator !=(const GroupVerificationStruct& a,
                const GroupVerificationStruct& b)
            {
                return !(a == b);
            }
        };

        class GroupVerificationStructHasher
        {
        public:
            GroupVerificationStructHasher() = default;
            ~GroupVerificationStructHasher() = default;

            size_t operator()(const GroupVerificationStruct& g) const
            {
                std::hash<long long> hasher;
                return hasher(g.group_id) * hasher(g.user_id);
            }
        };

        std::shared_ptr<BaseRoom>               m_globalRoom;

        // 群聊房间表
        std::unordered_map<long long,
            std::shared_ptr<qls::GroupRoom>>    m_baseRoom_map;
        mutable std::shared_mutex               m_baseRoom_map_mutex;

        // 私聊房间表
        std::unordered_map<long long,
            std::shared_ptr<qls::PrivateRoom>>  m_basePrivateRoom_map;
        mutable std::shared_mutex               m_basePrivateRoom_map_mutex;

        // 私聊用户对应私聊房间号表
        std::unordered_map<PrivateRoomIDStruct,
            long long,
            PrivateRoomIDStructHasher>          m_userID_to_privateRoomID_map;
        mutable std::shared_mutex               m_userID_to_privateRoomID_map_mutex;

        // 用户表
        std::unordered_map<long long,
            std::shared_ptr<qls::User>>         m_user_map;
        mutable std::shared_mutex               m_user_map_mutex;

        // 用户添加用户申请表
        std::unordered_map<PrivateRoomIDStruct,
            qls::FriendRoomVerification,
            PrivateRoomIDStructHasher>          m_FriendRoomVerification_map;
        mutable std::shared_mutex               m_FriendRoomVerification_map_mutex;

        // 用户添加群聊申请表
        std::unordered_map<GroupVerificationStruct,
            qls::GroupRoomVerification,
            GroupVerificationStructHasher>      m_GroupVerification_map;
        mutable std::shared_mutex               m_GroupVerification_map_mutex;

        // 新用户id
        std::atomic<long long>                  m_newUserId;
        // 新私聊房间id
        std::atomic<long long>                  m_newPrivateRoomId;
        // 新群聊房间id
        std::atomic<long long>                  m_newGroupRoomId;
        
        // sql进程管理
        quqisql::SQLDBProcess                   m_sqlProcess;
    };
}

#endif // !MANAGER_H
