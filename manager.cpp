#include "manager.h"

#include <stdexcept>
#include <Ini.h>

extern qini::INIObject serverIni;

namespace qls
{
    Manager::~Manager()
    {
    }

    void Manager::init()
    {
        // sql 初始化
        this->m_sqlProcess.setSQLServerInfo(serverIni["mysql"]["username"],
            serverIni["mysql"]["password"],
            "mysql",
            serverIni["mysql"]["host"],
            unsigned short(std::stoi(serverIni["mysql"]["port"])));

        this->m_sqlProcess.connectSQLServer();

        {
            this->m_newUserId = 10000;
            this->m_newPrivateRoomId = 10000;
            this->m_newGroupRoomId = 10000;

            // sql更新初始化数据
            // ...
        }
    }

    long long Manager::addPrivateRoom(long long user1_id, long long user2_id)
    {
        std::unique_lock<std::shared_mutex> ul1(this->m_basePrivateRoom_map_mutex, std::defer_lock),
            ul2(this->m_userID_to_privateRoomID_map_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        // 私聊房间id
        long long privateRoom_id = this->m_newGroupRoomId++;
        {
            /*
            * 这里有申请sql 创建私聊房间等命令
            */
        }

        this->m_basePrivateRoom_map[privateRoom_id] = std::make_shared<qls::PrivateRoom>(
            user1_id, user2_id);
        this->m_userID_to_privateRoomID_map[{user1_id, user2_id}] = privateRoom_id;
        
        // 初始化
        this->m_basePrivateRoom_map[privateRoom_id]->init();

        return privateRoom_id;
    }

    long long Manager::getPrivateRoomId(long long user1_id, long long user2_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_userID_to_privateRoomID_map_mutex);
        if (this->m_userID_to_privateRoomID_map.find(
            { user1_id , user2_id }) != this->m_userID_to_privateRoomID_map.end())
        {
            return this->m_userID_to_privateRoomID_map.find({ user1_id , user2_id })->second;
        }
        else if (this->m_userID_to_privateRoomID_map.find(
            { user2_id , user1_id }) != this->m_userID_to_privateRoomID_map.end())
        {
            return this->m_userID_to_privateRoomID_map.find({ user2_id , user1_id })->second;
        }
        else throw std::invalid_argument("there is not a room matches the argument");
    }

    bool Manager::hasPrivateRoom(long long private_room_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_basePrivateRoom_map_mutex);
        return this->m_basePrivateRoom_map.find(
            private_room_id) != this->m_basePrivateRoom_map.end();
    }

    std::shared_ptr<qls::PrivateRoom> Manager::getPrivateRoom(long long private_room_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_basePrivateRoom_map_mutex);
        auto itor = this->m_basePrivateRoom_map.find(private_room_id);
        if (itor == this->m_basePrivateRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");
        return itor->second;
    }

    void Manager::removePrivateRoom(long long private_room_id)
    {
        std::unique_lock<std::shared_mutex> ul1(this->m_basePrivateRoom_map_mutex, std::defer_lock),
            ul2(this->m_userID_to_privateRoomID_map_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        auto itor = this->m_basePrivateRoom_map.find(private_room_id);
        if (itor == this->m_basePrivateRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");

        {
            /*
            * 这里有申请sql 删除私聊房间等命令
            */
        }

        long long user1_id = itor->second->getUserID1();
        long long user2_id = itor->second->getUserID2();

        if (this->m_userID_to_privateRoomID_map.find(
            { user1_id , user2_id }) != this->m_userID_to_privateRoomID_map.end())
        {
            this->m_userID_to_privateRoomID_map.erase({ user1_id , user2_id });
        }
        else if (this->m_userID_to_privateRoomID_map.find(
            { user2_id , user1_id }) != this->m_userID_to_privateRoomID_map.end())
        {
            this->m_userID_to_privateRoomID_map.erase({ user2_id , user1_id });
        }

        this->m_basePrivateRoom_map.erase(itor);
    }

    long long Manager::addGroupRoom(long long opreator_user_id)
    {
        std::unique_lock<std::shared_mutex> lock(this->m_baseRoom_map_mutex);

        // 新群聊id
        long long group_room_id = this->m_newPrivateRoomId++;
        {
            /*
            * sql 创建群聊获取群聊id
            */
        }

        this->m_baseRoom_map[group_room_id] = std::make_shared<qls::GroupRoom>(group_room_id);

        // 初始化
        this->m_baseRoom_map[group_room_id]->init();

        return group_room_id;
    }

    bool Manager::hasGroupRoom(long long group_room_id) const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_baseRoom_map_mutex);
        return this->m_baseRoom_map.find(group_room_id) !=
            this->m_baseRoom_map.end();
    }

    std::shared_ptr<qls::GroupRoom> Manager::getGroupRoom(long long group_room_id) const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_baseRoom_map_mutex);

        auto itor = this->m_baseRoom_map.find(group_room_id);
        if (itor == this->m_baseRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");
        return itor->second;
    }

    void Manager::removeGroupRoom(long long group_room_id)
    {
        std::unique_lock<std::shared_mutex> lock(this->m_baseRoom_map_mutex);

        auto itor = this->m_baseRoom_map.find(group_room_id);
        if (itor == this->m_baseRoom_map.end())
            throw std::invalid_argument("there is not a room matches the argument");

        {
            /*
            * sql删除群聊
            */
        }

        this->m_baseRoom_map.erase(group_room_id);
    }

    std::shared_ptr<qls::User> Manager::addNewUser()
    {
        std::unique_lock<std::shared_mutex> ul(m_user_map_mutex);

        long long newUserId = m_newUserId++;
        {
            // sql处理数据
        }

        m_user_map[newUserId] = std::make_shared<qls::User>(newUserId);
        // 初始化
        m_user_map[newUserId]->init();

        return m_user_map[newUserId];
    }

    std::shared_ptr<qls::User> Manager::getUser(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);

        auto itor = m_user_map.find(user_id);
        if (itor == m_user_map.end())
            throw std::invalid_argument("there is not a user matches the argument");
        
        return itor->second;
    }

    void Manager::addFriendRoomVerification(long long user_id_1, long long user_id_2)
    {
        std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

        if (m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) ==
            m_FriendRoomVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        m_FriendRoomVerification_map.insert({ user_id_1, user_id_2 }, FriendRoomVerification{ user_id_1, user_id_2 });
    }

    bool Manager::hasFriendRoomVerification(long long user_id_1, long long user_id_2) const
    {
        std::shared_lock<std::shared_mutex> sl(m_FriendRoomVerification_map_mutex);
        
        return m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) !=
            m_FriendRoomVerification_map.end();
    }

    bool Manager::setFriendVerified(long long user_id_1, long long user_id_2, long long user_id, bool is_verified)
    {
        std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

        auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
        if (itor == m_FriendRoomVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        auto& ver = itor->second;
        ver.setUserVerified(user_id, is_verified);

        bool result = ver.getUserVerified(user_id_1) && ver.getUserVerified(user_id_2);

        if (result)
        {
            bool error = false;
            try
            {
                this->getPrivateRoomId(user_id_1, user_id_2);
                error = true;
            }
            catch (...)
            {
                std::unique_lock<std::shared_mutex> ul(m_basePrivateRoom_map_mutex);
                this->addPrivateRoom(user_id_1, user_id_2);
            }

            if (error) throw std::invalid_argument("Wrong argument!");
        }

        return result;
    }

    void Manager::addGroupRoomVerification(long long group_id, long long user_id)
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        if (m_GroupVerification_map.find({ group_id, user_id }) ==
            m_GroupVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        m_GroupVerification_map.insert({ group_id, user_id }, GroupRoomVerification{ group_id, user_id });
    }

    bool Manager::hasGroupRoomVerification(long long group_id, long long user_id)
    {
        std::shared_lock<std::shared_mutex> sl(m_GroupVerification_map_mutex);

        return m_GroupVerification_map.find({ group_id, user_id }) !=
            m_GroupVerification_map.end();
    }

    bool Manager::setGroupRoomGroupVerified(long long group_id, long long user_id, bool is_verified)
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        auto& ver = itor->second;
        ver.setGroupVerified(is_verified);
        bool result = ver.getGroupVerified() && ver.getUserVerified();
        if (result)
        {
            std::shared_lock<std::shared_mutex> sl(m_baseRoom_map_mutex);

            auto itor = m_baseRoom_map.find(group_id);
            if (itor == m_baseRoom_map.end())
                throw std::invalid_argument("Wrong argument!");

            itor->second->addMember(user_id);
        }
        return result;
    }

    bool Manager::setGroupRoomUserVerified(long long group_id, long long user_id, bool is_verified)
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        auto& ver = itor->second;
        ver.setUserVerified(is_verified);
        bool result = ver.getGroupVerified() && ver.getUserVerified();
        if (result)
        {
            std::shared_lock<std::shared_mutex> sl(m_baseRoom_map_mutex);

            auto itor = m_baseRoom_map.find(group_id);
            if (itor == m_baseRoom_map.end())
                throw std::invalid_argument("Wrong argument!");

            itor->second->addMember(user_id);
        }
        return result;
    }

    quqisql::SQLDBProcess& Manager::getServerSqlProcessor()
    {
        return this->m_sqlProcess;
    }
}
