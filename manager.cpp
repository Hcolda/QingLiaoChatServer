#include "manager.h"

#include <stdexcept>

namespace qls
{
    Manager& Manager::getGlobalManager()
    {
        static Manager localManager;
        return localManager;
    }

    void Manager::setSQLProcess(const std::shared_ptr<quqisql::SQLDBProcess>& process)
    {
        if (!process)
            throw std::invalid_argument("process is nullptr");

        m_sqlProcess = process;
    }

    long long Manager::addPrivateRoom(long long user1_id, long long user2_id)
    {
        std::unique_lock<std::shared_mutex> ul1(m_basePrivateRoom_map_mutex, std::defer_lock),
            ul2(m_userID_to_privateRoomID_map_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        // 私聊房间id
        long long privateRoom_id;
        {
            /*
            * 这里有申请sql 创建私聊房间等命令
            */

            privateRoom_id = 0;
        }

        m_basePrivateRoom_map[privateRoom_id] = std::make_shared<qls::BasePrivateRoom>(
            user1_id, user2_id);
        m_userID_to_privateRoomID_map[{user1_id, user2_id}] = privateRoom_id;
        
        return privateRoom_id;
    }

    long long Manager::getPrivateRoomId(long long user1_id, long long user2_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_userID_to_privateRoomID_map_mutex);
        if (m_userID_to_privateRoomID_map.find({ user1_id , user2_id }) != m_userID_to_privateRoomID_map.end())
        {
            return m_userID_to_privateRoomID_map.find({ user1_id , user2_id })->second;
        }
        else if (m_userID_to_privateRoomID_map.find({ user2_id , user1_id }) != m_userID_to_privateRoomID_map.end())
        {
            return m_userID_to_privateRoomID_map.find({ user2_id , user1_id })->second;
        }
        else throw std::invalid_argument("there is not a room matches the argument");
    }

    bool Manager::hasPrivateRoom(long long private_room_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_basePrivateRoom_map_mutex);
        return m_basePrivateRoom_map.find(private_room_id) != m_basePrivateRoom_map.end();
    }

    std::shared_ptr<qls::BasePrivateRoom> Manager::getPrivateRoom(long long private_room_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_basePrivateRoom_map_mutex);
        auto itor = m_basePrivateRoom_map.find(private_room_id);
        if (itor == m_basePrivateRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");
        return itor->second;
    }

    void Manager::removePrivateRoom(long long private_room_id)
    {
        std::unique_lock<std::shared_mutex> ul1(m_basePrivateRoom_map_mutex, std::defer_lock),
            ul2(m_userID_to_privateRoomID_map_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        auto itor = m_basePrivateRoom_map.find(private_room_id);
        if (itor == m_basePrivateRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");

        {
            /*
            * 这里有申请sql 删除私聊房间等命令
            */
        }

        long long user1_id = itor->second->getUserID1();
        long long user2_id = itor->second->getUserID2();

        if (m_userID_to_privateRoomID_map.find({ user1_id , user2_id }) != m_userID_to_privateRoomID_map.end())
        {
            m_userID_to_privateRoomID_map.erase({ user1_id , user2_id });
        }
        else if (m_userID_to_privateRoomID_map.find({ user2_id , user1_id }) != m_userID_to_privateRoomID_map.end())
        {
            m_userID_to_privateRoomID_map.erase({ user2_id , user1_id });
        }

        m_basePrivateRoom_map.erase(itor);
    }

    void Manager::addGroupRoom(long long group_room_id)
    {
    }

    bool Manager::hasGroupRoom(long long private_room_id) const
    {
        return false;
    }

    std::shared_ptr<qls::BaseGroupRoom> Manager::getGroupRoom(long long group_room_id) const
    {
        return std::shared_ptr<qls::BaseGroupRoom>();
    }

    void Manager::removeGroupRoom(long long group_room_id)
    {
    }

    void Manager::init()
    {
    }
}
