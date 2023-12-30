#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

#include "definition.hpp"
#include "room.h"

namespace qls
{
    class Manager
    {
    protected:
        Manager();
        ~Manager() = default;

    public:
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
        * @return class BasePrivateRoom
        */
        std::shared_ptr<qls::BasePrivateRoom> getPrivateRoom(long long private_room_id) const;
        
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
        * @return class BaseGroupRoom
        */
        std::shared_ptr<qls::BaseGroupRoom> getGroupRoom(long long group_room_id) const;
        
        /*
        * @brief 删除群聊房间
        * @param group_room_id 群聊房间id
        */
        void removeGroupRoom(long long group_room_id);

        /*
        * @brief 初始化
        */
        void init();

    private:
        struct ManagerImpl;

        std::unique_ptr<ManagerImpl> m_manager_impl;
    };
}
