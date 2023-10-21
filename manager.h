#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

#include "definition.hpp"
#include "room.h"
#include "SQLProcess.hpp"

namespace qls
{
    class Manager
    {
    protected:
        Manager() = default;
        ~Manager() = default;

    public:
        /*
        * @brief 获取全局管理器
        * @return 管理器
        */
        static Manager& getGlobalManager();

        /*
        * @brief 设置sql
        * @param process sqlProcess
        */
        void setSQLProcess(const std::shared_ptr<quqisql::SQLDBProcess>& process);

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
        void addGroupRoom(long long group_room_id);
        
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
        std::shared_ptr<quqisql::SQLDBProcess>                                  m_sqlProcess;

        std::unordered_map<long long, std::shared_ptr<qls::BaseGroupRoom>>      m_baseRoom_map;
        mutable std::shared_mutex                                               m_baseRoom_map_mutex;

        std::unordered_map<long long, std::shared_ptr<qls::BasePrivateRoom>>    m_basePrivateRoom_map;
        mutable std::shared_mutex                                               m_basePrivateRoom_map_mutex;

        std::unordered_map<std::pair<long long, long long>, long long>          m_userID_to_privateRoomID_map;
        mutable std::shared_mutex                                               m_userID_to_privateRoomID_map_mutex;
    };
}
