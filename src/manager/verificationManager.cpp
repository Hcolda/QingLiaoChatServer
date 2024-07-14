#include "verificationManager.h"

#include <mutex>
#include <shared_mutex>

#include "user.h"
#include "manager.h"

// manager
extern qls::Manager serverManager;

namespace qls
{

void VerificationManager::init()
{
    // sql init
}

void VerificationManager::addFriendRoomVerification(long long user_id_1, long long user_id_2)
{
    {
        std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

        if (m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) ==
            m_FriendRoomVerification_map.end())
            throw std::invalid_argument("The same verification has existed!");

        m_FriendRoomVerification_map.emplace(PrivateRoomIDStruct{ user_id_1, user_id_2 },
                                                FriendRoomVerification{ user_id_1, user_id_2 });
    }
    
    // user1
    {
        qls::UserVerificationStruct uv;

        uv.user_id = user_id_2;
        uv.verification_type =
            qls::UserVerificationStruct::VerificationType::Sent;

        auto ptr = serverManager.getUser(user_id_1);
        ptr->addFriendVerification(user_id_2, std::move(uv));

        // 未完成 通知另一方
        // m_globalRoom->baseSendData()
    }

    // user2
    {
        qls::UserVerificationStruct uv;

        uv.user_id = user_id_1;
        uv.verification_type =
            qls::UserVerificationStruct::VerificationType::Received;

        auto ptr = serverManager.getUser(user_id_2);
        ptr->addFriendVerification(user_id_1, std::move(uv));

        // 未完成 通知另一方
        // m_globalRoom->baseSendData()
    }
}

bool VerificationManager::hasFriendRoomVerification(long long user_id_1, long long user_id_2) const
{
    std::shared_lock<std::shared_mutex> sl(m_FriendRoomVerification_map_mutex);
    
    return m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) !=
        m_FriendRoomVerification_map.end();
}

bool VerificationManager::setFriendVerified(long long user_id_1, long long user_id_2,
                                long long user_id)
{
    bool result = false;
    {
        std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

        auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
        if (itor == m_FriendRoomVerification_map.end())
            throw std::invalid_argument("The same verification has existed!");

        auto& ver = itor->second;
        ver.setUserVerified(user_id);

        result = ver.getUserVerified(user_id_1) && ver.getUserVerified(user_id_2);
    }

    if (result)
    {
        bool error = false;
        try
        {
            serverManager.getPrivateRoomId(user_id_1, user_id_2);
            error = true;
        }
        catch (...)
        {
            serverManager.addPrivateRoom(user_id_1, user_id_2);

            // 更新user1的friendList
            {
                if (!serverManager.hasUser(user_id_1))
                    throw std::invalid_argument("Wrong argument!");
                auto ptr = serverManager.getUser(user_id_1);
                auto set = std::move(ptr->getFriendList());
                set.insert(user_id_2);
                ptr->updateFriendList(std::move(set));

                ptr->removeFriendVerification(user_id_2);
            }
            
            // 更新user2的friendList
            {
                if (!serverManager.hasUser(user_id_2))
                    throw std::invalid_argument("Wrong argument!");
                auto ptr = serverManager.getUser(user_id_2);
                auto set = std::move(ptr->getFriendList());
                set.insert(user_id_1);
                ptr->updateFriendList(std::move(set));

                ptr->removeFriendVerification(user_id_1);
            }

            // 未完成 通知另一方
            // m_globalRoom->baseSendData()
        }

        if (error) throw std::invalid_argument("Wrong argument!");
    }

    return result;
}

void VerificationManager::removeFriendRoomVerification(long long user_id_1, long long user_id_2)
{
    {
        std::unique_lock<std::shared_mutex> ul(m_FriendRoomVerification_map_mutex);

        auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
        if (itor == m_FriendRoomVerification_map.end())
            throw std::invalid_argument("The same verification has existed!");

        m_FriendRoomVerification_map.erase(itor);
    }

    serverManager.getUser(user_id_1)->removeFriendVerification(user_id_2);
    serverManager.getUser(user_id_2)->removeFriendVerification(user_id_1);
}

void VerificationManager::addGroupRoomVerification(long long group_id, long long user_id)
{
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        if (m_GroupVerification_map.find({ group_id, user_id }) ==
            m_GroupVerification_map.end())
            throw std::invalid_argument("The same verification has existed!");

        m_GroupVerification_map.emplace(GroupVerificationStruct{ group_id, user_id },
                                        GroupRoomVerification{ group_id, user_id });
    }

    // 用户发送请求
    {
        qls::UserVerificationStruct uv;

        uv.user_id = group_id;
        uv.verification_type =
            qls::UserVerificationStruct::VerificationType::Sent;

        auto ptr = serverManager.getUser(user_id);
        ptr->addGroupVerification(group_id, std::move(uv));
    }

    // 群聊拥有者接收请求
    {
        qls::UserVerificationStruct uv;

        uv.user_id = user_id;
        uv.verification_type =
            qls::UserVerificationStruct::VerificationType::Received;

        auto ptr = serverManager.getUser(serverManager.getGroupRoom(group_id)->getAdministrator());
        ptr->addGroupVerification(group_id, std::move(uv));
    }

    // 未完成 通知另一方
    // m_globalRoom->baseSendData()
}

bool VerificationManager::hasGroupRoomVerification(long long group_id, long long user_id) const
{
    std::shared_lock<std::shared_mutex> sl(m_GroupVerification_map_mutex);

    return m_GroupVerification_map.find({ group_id, user_id }) !=
        m_GroupVerification_map.end();
}

bool VerificationManager::setGroupRoomGroupVerified(long long group_id, long long user_id)
{
    bool result = false;
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        auto& ver = itor->second;
        ver.setGroupVerified();
        result = ver.getGroupVerified() && ver.getUserVerified();
    }

    if (result)
    {
        if (!serverManager.hasGroupRoom(group_id))
            throw std::invalid_argument("Wrong argument!");
        serverManager.getGroupRoom(group_id)->addMember(user_id);

        // 更新user的groupList
        if (!serverManager.hasUser(user_id))
            throw std::invalid_argument("Wrong argument!");
        auto ptr = serverManager.getUser(user_id);
        auto set = std::move(ptr->getGroupList());
        set.insert(group_id);
        ptr->updateGroupList(std::move(set));

        // 未完成 通知另一方
        // m_globalRoom->baseSendData()
    }
    return result;
}

bool VerificationManager::setGroupRoomUserVerified(long long group_id, long long user_id)
{
    bool result = false;
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        auto& ver = itor->second;
        ver.setUserVerified();
        result = ver.getGroupVerified() && ver.getUserVerified();
    }

    if (result)
    {
        if (!serverManager.hasGroupRoom(group_id))
            throw std::invalid_argument("Wrong argument!");
        serverManager.getGroupRoom(group_id)->addMember(user_id);

        // 更新user的friendlist
        if (!serverManager.hasUser(user_id))
            throw std::invalid_argument("Wrong argument!");

        {
            auto ptr = serverManager.getUser(user_id);
            auto set = std::move(ptr->getGroupList());
            set.insert(group_id);
            ptr->updateGroupList(std::move(set));

            ptr->removeGroupVerification(group_id, user_id);
        }

        serverManager.getUser(serverManager.getGroupRoom(group_id)->getAdministrator())
            ->removeGroupVerification(group_id, user_id);
    }
    return result;
}

void VerificationManager::removeGroupRoomVerification(long long group_id, long long user_id)
{
    {
        std::unique_lock<std::shared_mutex> ul(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.end())
            throw std::invalid_argument("Wrong argument!");

        m_GroupVerification_map.erase(itor);
    }

    serverManager.getUser(serverManager.getGroupRoom(group_id)->getAdministrator())
        ->removeGroupVerification(group_id, user_id);

    serverManager.getUser(user_id)->removeGroupVerification(group_id, user_id);
}

} // namespace qls
