#include "manager.h"

#include <stdexcept>
#include <Ini.h>

#include "user.h"

extern qini::INIObject serverIni;

namespace qls
{
    Manager::~Manager()
    {
    }

    void Manager::init()
    {
        // sql 初始化
        /*this->m_sqlProcess.setSQLServerInfo(serverIni["mysql"]["username"],
            serverIni["mysql"]["password"],
            "mysql",
            serverIni["mysql"]["host"],
            unsigned short(std::stoi(serverIni["mysql"]["port"])));

        this->m_sqlProcess.connectSQLServer();*/

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

    void Manager::addFriendRoomVerification(long long user_id_1, long long user_id_2)
    {
        {
            std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

            if (m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) ==
                m_FriendRoomVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            m_FriendRoomVerification_map.emplace(PrivateRoomIDStruct{ user_id_1, user_id_2 },
                                                 FriendRoomVerification{ user_id_1, user_id_2 });
        }
        
        // user1
        {
            qls::User::UserVerificationStruct uv;

            uv.user_id = user_id_2;
            uv.verification_type =
                qls::User::UserVerificationStruct::VerificationType::Sent;

            auto ptr = this->getUser(user_id_1);
            ptr->addFriendVerification(user_id_2, std::move(uv));
        }

        // user2
        {
            qls::User::UserVerificationStruct uv;

            uv.user_id = user_id_1;
            uv.verification_type =
                qls::User::UserVerificationStruct::VerificationType::Received;

            auto ptr = this->getUser(user_id_2);
            ptr->addFriendVerification(user_id_1, std::move(uv));
        }
    }

    bool Manager::hasFriendRoomVerification(long long user_id_1, long long user_id_2) const
    {
        std::shared_lock<std::shared_mutex> sl(m_FriendRoomVerification_map_mutex);
        
        return m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) !=
            m_FriendRoomVerification_map.end();
    }

    bool Manager::setFriendVerified(long long user_id_1, long long user_id_2,
                                    long long user_id, bool is_verified)
    {
        bool result = false;
        {
            std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

            auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
            if (itor == m_FriendRoomVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            auto& ver = itor->second;
            ver.setUserVerified(user_id, is_verified);

            result = ver.getUserVerified(user_id_1) && ver.getUserVerified(user_id_2);
        }

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

                // 更新user1的friendList
                {
                    std::shared_ptr<qls::User> ptr;
                    {
                        std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);
                        auto itor = m_user_map.find(user_id_1);
                        if (itor == m_user_map.end())
                            throw std::invalid_argument("Wrong argument!");
                        ptr = itor->second;
                    }
                    auto set = std::move(ptr->getFriendList());
                    set.insert(user_id_2);
                    ptr->updateFriendList(std::move(set));

                    ptr->removeFriendVerification(user_id_2);
                }
                
                // 更新user2的friendList
                {
                    std::shared_ptr<qls::User> ptr;
                    {
                        std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);
                        auto itor = m_user_map.find(user_id_2);
                        if (itor == m_user_map.end())
                            throw std::invalid_argument("Wrong argument!");
                        ptr = itor->second;
                    }
                    auto set = std::move(ptr->getFriendList());
                    set.insert(user_id_1);
                    ptr->updateFriendList(std::move(set));

                    ptr->removeFriendVerification(user_id_1);
                }
            }

            if (error) throw std::invalid_argument("Wrong argument!");
        }

        return result;
    }

    void Manager::removeFriendRoomVerification(long long user_id_1, long long user_id_2)
    {
        {
            std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

            auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
            if (itor == m_FriendRoomVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            m_FriendRoomVerification_map.erase(itor);
        }

        this->getUser(user_id_1)->removeFriendVerification(user_id_2);
        this->getUser(user_id_2)->removeFriendVerification(user_id_1);
    }

    void Manager::addGroupRoomVerification(long long group_id, long long user_id)
    {
        {
            std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

            if (m_GroupVerification_map.find({ group_id, user_id }) ==
                m_GroupVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            m_GroupVerification_map.emplace(GroupVerificationStruct{ group_id, user_id },
                                            GroupRoomVerification{ group_id, user_id });
        }

        // 用户发送请求
        {
            qls::User::UserVerificationStruct uv;

            uv.user_id = group_id;
            uv.verification_type =
                qls::User::UserVerificationStruct::VerificationType::Sent;

            auto ptr = this->getUser(user_id);
            ptr->addGroupVerification(group_id, std::move(uv));
        }

        // 群聊拥有者接收请求
        {
            qls::User::UserVerificationStruct uv;

            uv.user_id = user_id;
            uv.verification_type =
                qls::User::UserVerificationStruct::VerificationType::Received;

            auto ptr = this->getUser(this->getGroupRoom(group_id)->getAdministrator());
            ptr->addGroupVerification(group_id, std::move(uv));
        }
    }

    bool Manager::hasGroupRoomVerification(long long group_id, long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_GroupVerification_map_mutex);

        return m_GroupVerification_map.find({ group_id, user_id }) !=
            m_GroupVerification_map.end();
    }

    bool Manager::setGroupRoomGroupVerified(long long group_id, long long user_id, bool is_verified)
    {
        bool result = false;
        {
            std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

            auto itor = m_GroupVerification_map.find({ group_id, user_id });
            if (itor == m_GroupVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            auto& ver = itor->second;
            ver.setGroupVerified(is_verified);
            result = ver.getGroupVerified() && ver.getUserVerified();
        }

        if (result)
        {
            std::shared_lock<std::shared_mutex> sl(m_baseRoom_map_mutex);

            auto itor = m_baseRoom_map.find(group_id);
            if (itor == m_baseRoom_map.end())
                throw std::invalid_argument("Wrong argument!");

            itor->second->addMember(user_id);

            // 更新user的groupList
            std::shared_ptr<qls::User> ptr;
            {
                std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);
                auto itor = m_user_map.find(user_id);
                if (itor == m_user_map.end())
                    throw std::invalid_argument("Wrong argument!");
                ptr = itor->second;
            }
            auto set = std::move(ptr->getGroupList());
            set.insert(group_id);
            ptr->updateGroupList(std::move(set));
        }
        return result;
    }

    bool Manager::setGroupRoomUserVerified(long long group_id, long long user_id, bool is_verified)
    {
        bool result = false;
        {
            std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

            auto itor = m_GroupVerification_map.find({ group_id, user_id });
            if (itor == m_GroupVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            auto& ver = itor->second;
            ver.setUserVerified(is_verified);
            result = ver.getGroupVerified() && ver.getUserVerified();
        }

        if (result)
        {
            std::shared_lock<std::shared_mutex> sl(m_baseRoom_map_mutex);

            auto itor = m_baseRoom_map.find(group_id);
            if (itor == m_baseRoom_map.end())
                throw std::invalid_argument("Wrong argument!");

            itor->second->addMember(user_id);

            // 更新user的friendlist
            std::shared_ptr<qls::User> ptr;
            {
                std::shared_lock<std::shared_mutex> sl(m_user_map_mutex);
                auto itor = m_user_map.find(user_id);
                if (itor == m_user_map.end())
                    throw std::invalid_argument("Wrong argument!");
                ptr = itor->second;
            }
            auto set = std::move(ptr->getGroupList());
            set.insert(group_id);
            ptr->updateGroupList(std::move(set));

            ptr->removeGroupVerification(group_id, user_id);

            {
                auto ptr = this->getUser(this->getGroupRoom(group_id)->getAdministrator());
                ptr->removeGroupVerification(group_id, user_id);
            }
        }
        return result;
    }

    void Manager::removeGroupRoomVerification(long long group_id, long long user_id)
    {
        {
            std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

            auto itor = m_GroupVerification_map.find({ group_id, user_id });
            if (itor == m_GroupVerification_map.end())
                throw std::invalid_argument("Wrong argument!");

            m_GroupVerification_map.erase(itor);
        }

        this->getUser(getGroupRoom(group_id)->getAdministrator())
            ->removeGroupVerification(group_id, user_id);

        this->getUser(user_id)->removeGroupVerification(group_id, user_id);
    }

    quqisql::SQLDBProcess& Manager::getServerSqlProcess()
    {
        return this->m_sqlProcess;
    }
}
