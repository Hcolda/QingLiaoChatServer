#include "manager.h"

#include <stdexcept>
#include <Ini.h>

#include "user.h"
#include "dataPackage.h"

extern qini::INIObject serverIni;

namespace qls
{
    Manager::~Manager()
    {
    }

    void Manager::init()
    {
        // sql 初始化
        // this->m_sqlProcess.setSQLServerInfo(serverIni["mysql"]["username"],
        //     serverIni["mysql"]["password"],
        //     "mysql",
        //     serverIni["mysql"]["host"],
        //     unsigned short(std::stoi(serverIni["mysql"]["port"])));

        // this->m_sqlProcess.connectSQLServer();

        {
            this->m_newUserId = 10000;
            this->m_newPrivateRoomId = 10000;
            this->m_newGroupRoomId = 10000;

            // sql更新初始化数据
            // ...
        }

        m_dataManager.init();
        m_verificationManager.init();
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
            user1_id, user2_id, true);
        this->m_userID_to_privateRoomID_map[{user1_id, user2_id}] = privateRoom_id;

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

        this->m_baseRoom_map[group_room_id] = std::make_shared<qls::GroupRoom>(
            group_room_id, opreator_user_id, true);
        this->m_baseRoom_map[group_room_id]->setAdministrator(opreator_user_id);

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

        m_user_map[newUserId] = std::make_shared<qls::User>(newUserId, true);

        return m_user_map[newUserId];
    }

    bool Manager::hasUser(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);

        return m_user_map.find(user_id) != m_user_map.end();
    }

    std::shared_ptr<qls::User> Manager::getUser(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);

        auto itor = m_user_map.find(user_id);
        if (itor == m_user_map.end())
            throw std::invalid_argument("there is not a user matches the argument");
        
        return itor->second;
    }

    std::unordered_map<long long, std::shared_ptr<qls::User>> Manager::getUserList() const
    {
        std::shared_lock lock(m_user_map_mutex);
        return m_user_map;
    }

    quqisql::SQLDBProcess& Manager::getServerSqlProcess()
    {
        return this->m_sqlProcess;
    }

    qls::DataManager &Manager::getServerDataManager()
    {
        return m_dataManager;
    }

    qls::VerificationManager &Manager::getServerVerificationManager()
    {
        return m_verificationManager;
    }
}
