#pragma once

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
        ~Manager();

        /*
        * @brief 初始化
        */
        void init();

        /*
        * @brief 添加私聊房间
        * @param user1_id 用户1ID
        * @param user2_id 用户2ID
        * @return 创建后的私聊房间ID
        */
        long long addPrivateRoom(long long user1_id, long long user2_id);

        /*
        * @brief 获取私聊房间ID
        * @param user1_id 用户1ID
        * @param user2_id 用户2ID
        * @return 私聊房间ID
        */
        long long getPrivateRoomId(long long user1_id, long long user2_id) const;
        
        /*
        * @brief 是否有此私聊房间
        * @param private_room_id 私聊房间ID
        * @return true 有 | false 无
        */
        bool hasPrivateRoom(long long private_room_id) const;
        
        /*
        * @brief 获取私聊房间
        * @param private_room_id 私聊房间ID
        * @return class PrivateRoom
        */
        std::shared_ptr<qls::PrivateRoom> getPrivateRoom(long long private_room_id) const;
        
        /*
        * @brief 删除私聊房间
        * @param private_room_id 私聊房间ID
        */
        void removePrivateRoom(long long private_room_id);

        /*
        * @brief 添加群聊房间
        * @param group_room_id 群聊房间id
        */
        long long addGroupRoom(long long opreator_user_id);
        
        /*
        * @brief 是否有群聊房间
        * @param group_room_id 群聊房间id
        * @return true 有 | false 无
        */
        bool hasGroupRoom(long long group_room_id) const;
        
        /*
        * @brief 获取群聊房间
        * @param group_room_id 群聊房间id
        * @return class GroupRoom
        */
        std::shared_ptr<qls::GroupRoom> getGroupRoom(long long group_room_id) const;
        
        /*
        * @brief 删除群聊房间
        * @param group_room_id 群聊房间id
        */
        void removeGroupRoom(long long group_room_id);

        /*
        * @brief 创建新用户
        * @return 新用户的user类
        */
        std::shared_ptr<qls::User> addNewUser();

        /*
        * @brief 获取用户类
        * @return user类
        */
        std::shared_ptr<qls::User> getUser(long long user_id) const;

        /*
        * @brief 添加私聊房间前的验证
        * @param user_id_1 用户1 id
        * @param user_id_2 用户2 id
        */
        void addFriendRoomVerification(long long user_id_1, long long user_id_2);

        /*
        * @brief 是否拥有私聊房间验证
        * @param user_id_1 用户1 id
        * @param user_id_2 用户2 id
        * @return true 拥有 | false 不拥有
        */
        bool hasFriendRoomVerification(long long user_id_1, long long user_id_2) const;

        /*
        * @brief 设置用户是否验证
        * @param user_id_1 用户1 id
        * @param user_id_2 用户2 id
        * @param user_id 设置的用户id
        * @param is_verified 是否验证
        * @return true 双方都验证(自动创建房间) | false 还有人没有验证
        */
        bool setFriendVerified(long long user_id_1, long long user_id_2, long long user_id, bool is_verified);

        /*
        * @brief 添加群聊验证
        * @param group_id 群聊id
        * @param user_id 用户id
        */
        void addGroupRoomVerification(long long group_id, long long user_id);

        /*
        * @brief 是否有验证
        * @param group_id 群聊id
        * @param user_id 用户id
        * @return true 拥有 | false 不拥有
        */
        bool hasGroupRoomVerification(long long group_id, long long user_id);

        /*
        * @brief 设置群聊验证的群聊是否验证
        * @param group_id 群聊id
        * @param user_id 用户id
        * @param is_verified 是否验证
        * @return true 双方都验证(用户自动加入群聊房间) | false 还有一方没有验证
        */
        bool setGroupRoomGroupVerified(long long group_id, long long user_id, bool is_verified);
        
        /*
        * @brief 设置群聊验证的用户是否验证
        * @param group_id 群聊id
        * @param user_id 用户id
        * @param is_verified 是否验证
        * @return true 双方都验证(用户自动加入群聊房间) | false 还有一方没有验证
        */
        bool setGroupRoomUserVerified(long long group_id, long long user_id, bool is_verified);

        /*
        * @brief 获取服务器的sql处理器
        */
        quqisql::SQLDBProcess& getServerSqlProcessor();

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

        std::unordered_map<long long,
            std::shared_ptr<qls::GroupRoom>>    m_baseRoom_map;
        mutable std::shared_mutex               m_baseRoom_map_mutex;

        std::unordered_map<long long,
            std::shared_ptr<qls::PrivateRoom>>  m_basePrivateRoom_map;
        mutable std::shared_mutex               m_basePrivateRoom_map_mutex;

        std::unordered_map<PrivateRoomIDStruct,
            long long,
            PrivateRoomIDStructHasher>          m_userID_to_privateRoomID_map;
        mutable std::shared_mutex               m_userID_to_privateRoomID_map_mutex;

        std::unordered_map<long long,
            std::shared_ptr<qls::User>>         m_user_map;
        mutable std::shared_mutex               m_user_map_mutex;

        std::unordered_map<PrivateRoomIDStruct,
            qls::FriendRoomVerification,
            PrivateRoomIDStructHasher>          m_FriendRoomVerification_map;
        mutable std::shared_mutex               m_FriendRoomVerification_map_mutex;

        std::unordered_map<GroupVerificationStruct,
            qls::GroupRoomVerification,
            GroupVerificationStructHasher>      m_GroupVerification_map;
        mutable std::shared_mutex               m_GroupVerification_map_mutex;

        std::atomic<long long>                  m_newUserId;
        std::atomic<long long>                  m_newPrivateRoomId;
        std::atomic<long long>                  m_newGroupRoomId;
    
        quqisql::SQLDBProcess                   m_sqlProcess;
    };
}
