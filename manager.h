#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "SQLProcess.hpp"
#include "definition.hpp"
#include "room.h"
#include "user.h"

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
        * @return 新用户的user_id
        */
        long long addNewUser();

        std::shared_ptr<qls::User> getUser(long long user_id) const;

        /*
        * @brief 获取服务器的sql处理器
        */
        quqisql::SQLDBProcess& getServerSqlProcessor();

    private:
        struct PrivateRoomIDStruct
        {
            long long user_id_1;
            long long user_id_2;

            friend bool operator ==(const PrivateRoomIDStruct& a, const PrivateRoomIDStruct& b)
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

        std::atomic<long long>                  m_newUserId;
        std::atomic<long long>                  m_newPrivateRoomId;
        std::atomic<long long>                  m_newGroupRoomId;
    
        quqisql::SQLDBProcess                   m_sqlProcess;
    };
}
