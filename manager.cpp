#include "manager.h"

#include <stdexcept>

namespace qls
{
    struct Manager::ManagerImpl
    {
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
            size_t operator ()(const PrivateRoomIDStruct& s) const
            {
                std::hash<long long> hasher;
                return hasher(s.user_id_1) * hasher(s.user_id_2);
            }
        };

        std::unordered_map<long long,
            std::shared_ptr<qls::BaseGroupRoom>>    m_baseRoom_map;
        mutable std::shared_mutex                   m_baseRoom_map_mutex;

        std::unordered_map<long long,
            std::shared_ptr<qls::BasePrivateRoom>>  m_basePrivateRoom_map;
        mutable std::shared_mutex                   m_basePrivateRoom_map_mutex;

        std::unordered_map<PrivateRoomIDStruct,
            long long,
            PrivateRoomIDStructHasher>              m_userID_to_privateRoomID_map;
        mutable std::shared_mutex                   m_userID_to_privateRoomID_map_mutex;
    };

    Manager::Manager()
    {
        m_manager_impl = std::make_unique<ManagerImpl>();
    }

    long long Manager::addPrivateRoom(long long user1_id, long long user2_id)
    {
        std::unique_lock<std::shared_mutex> ul1(m_manager_impl->m_basePrivateRoom_map_mutex, std::defer_lock),
            ul2(m_manager_impl->m_userID_to_privateRoomID_map_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        // 私聊房间id
        long long privateRoom_id;
        {
            /*
            * 这里有申请sql 创建私聊房间等命令
            */

            privateRoom_id = 0;
        }

        m_manager_impl->m_basePrivateRoom_map[privateRoom_id] = std::make_shared<qls::BasePrivateRoom>(
            user1_id, user2_id);
        m_manager_impl->m_userID_to_privateRoomID_map[{user1_id, user2_id}] = privateRoom_id;
        
        return privateRoom_id;
    }

    long long Manager::getPrivateRoomId(long long user1_id, long long user2_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->m_userID_to_privateRoomID_map_mutex);
        if (m_manager_impl->m_userID_to_privateRoomID_map.find(
            { user1_id , user2_id }) != m_manager_impl->m_userID_to_privateRoomID_map.end())
        {
            return m_manager_impl->m_userID_to_privateRoomID_map.find({ user1_id , user2_id })->second;
        }
        else if (m_manager_impl->m_userID_to_privateRoomID_map.find(
            { user2_id , user1_id }) != m_manager_impl->m_userID_to_privateRoomID_map.end())
        {
            return m_manager_impl->m_userID_to_privateRoomID_map.find({ user2_id , user1_id })->second;
        }
        else throw std::invalid_argument("there is not a room matches the argument");
    }

    bool Manager::hasPrivateRoom(long long private_room_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->m_basePrivateRoom_map_mutex);
        return m_manager_impl->m_basePrivateRoom_map.find(
            private_room_id) != m_manager_impl->m_basePrivateRoom_map.end();
    }

    std::shared_ptr<qls::BasePrivateRoom> Manager::getPrivateRoom(long long private_room_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->m_basePrivateRoom_map_mutex);
        auto itor = m_manager_impl->m_basePrivateRoom_map.find(private_room_id);
        if (itor == m_manager_impl->m_basePrivateRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");
        return itor->second;
    }

    void Manager::removePrivateRoom(long long private_room_id)
    {
        std::unique_lock<std::shared_mutex> ul1(m_manager_impl->m_basePrivateRoom_map_mutex, std::defer_lock),
            ul2(m_manager_impl->m_userID_to_privateRoomID_map_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        auto itor = m_manager_impl->m_basePrivateRoom_map.find(private_room_id);
        if (itor == m_manager_impl->m_basePrivateRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");

        {
            /*
            * 这里有申请sql 删除私聊房间等命令
            */
        }

        long long user1_id = itor->second->getUserID1();
        long long user2_id = itor->second->getUserID2();

        if (m_manager_impl->m_userID_to_privateRoomID_map.find(
            { user1_id , user2_id }) != m_manager_impl->m_userID_to_privateRoomID_map.end())
        {
            m_manager_impl->m_userID_to_privateRoomID_map.erase({ user1_id , user2_id });
        }
        else if (m_manager_impl->m_userID_to_privateRoomID_map.find(
            { user2_id , user1_id }) != m_manager_impl->m_userID_to_privateRoomID_map.end())
        {
            m_manager_impl->m_userID_to_privateRoomID_map.erase({ user2_id , user1_id });
        }

        m_manager_impl->m_basePrivateRoom_map.erase(itor);
    }

    long long Manager::addGroupRoom(long long opreator_user_id)
    {
        std::unique_lock<std::shared_mutex> lock(m_manager_impl->m_baseRoom_map_mutex);

        // 新群聊id
        long long group_room_id = 0;
        {
            /*
            * sql 创建群聊获取群聊id
            */
        }

        m_manager_impl->m_baseRoom_map[group_room_id] = std::make_shared<qls::BaseGroupRoom>(group_room_id);

        return group_room_id;
    }

    bool Manager::hasGroupRoom(long long group_room_id) const
    {
        std::shared_lock<std::shared_mutex> lock(m_manager_impl->m_baseRoom_map_mutex);
        return m_manager_impl->m_baseRoom_map.find(group_room_id) !=
            m_manager_impl->m_baseRoom_map.end();
    }

    std::shared_ptr<qls::BaseGroupRoom> Manager::getGroupRoom(long long group_room_id) const
    {
        std::shared_lock<std::shared_mutex> lock(m_manager_impl->m_baseRoom_map_mutex);

        auto itor = m_manager_impl->m_baseRoom_map.find(group_room_id);
        if (itor == m_manager_impl->m_baseRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");
        return itor->second;
    }

    void Manager::removeGroupRoom(long long group_room_id)
    {
        std::unique_lock<std::shared_mutex> lock(m_manager_impl->m_baseRoom_map_mutex);

        auto itor = m_manager_impl->m_baseRoom_map.find(group_room_id);
        if (itor == m_manager_impl->m_baseRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");

        {
            /*
            * sql删除群聊
            */
        }

        m_manager_impl->m_baseRoom_map.erase(group_room_id);
    }

    void Manager::init()
    {
    }
}
