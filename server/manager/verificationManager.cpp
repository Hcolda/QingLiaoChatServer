#include "verificationManager.h"

#include <mutex>
#include <shared_mutex>
#include <Json.h>
#include <dataPackage.h>

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

void VerificationManager::addFriendRoomVerification(UserID user_id_1, UserID user_id_2)
{
    if (user_id_1 == user_id_2)
        throw std::system_error(make_error_code(qls_errc::invalid_verification));

    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_FriendRoomVerification_map_mutex);

        if (m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) !=
            m_FriendRoomVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_existed));

        m_FriendRoomVerification_map.emplace(PrivateRoomIDStruct{ user_id_1, user_id_2 },
                                             FriendRoomVerification{ user_id_1, user_id_2 });
    }
    
    // user1
    {
        qls::UserVerificationStructure uv;

        uv.user_id = user_id_2;
        uv.verification_type =
            qls::UserVerificationStructure::VerificationType::Sent;

        auto ptr = serverManager.getUser(user_id_1);
        ptr->addFriendVerification(user_id_2, std::move(uv));
    }

    // user2
    {
        qls::UserVerificationStructure uv;

        uv.user_id = user_id_1;
        uv.verification_type =
            qls::UserVerificationStructure::VerificationType::Received;

        auto ptr = serverManager.getUser(user_id_2);
        ptr->addFriendVerification(user_id_1, std::move(uv));

        // notify the other successfully adding a friend
        qjson::JObject json(qjson::JValueType::JDict);
        json["userid"] = user_id_1.getOriginValue();
        json["type"] = "added_friend_verfication";
        auto pack = DataPackage::makePackage(qjson::JWriter::fastWrite(json));
        pack->type = 1;
        serverManager.getUser(user_id_2)->notifyAll(pack->packageToString());
    }
}

bool VerificationManager::hasFriendRoomVerification(UserID user_id_1, UserID user_id_2) const
{
    if (user_id_1 == user_id_2)
        throw std::system_error(make_error_code(qls_errc::invalid_verification));

    std::shared_lock<std::shared_mutex> local_shared_lock(m_FriendRoomVerification_map_mutex);
    
    return m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) !=
        m_FriendRoomVerification_map.end();
}

bool VerificationManager::setFriendVerified(UserID user_id_1, UserID user_id_2,
                                            UserID user_id)
{
    if (user_id_1 == user_id_2)
        throw std::system_error(make_error_code(qls_errc::invalid_verification));
    
    bool result = false;
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_FriendRoomVerification_map_mutex);

        auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
        if (itor == m_FriendRoomVerification_map.end())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        auto& ver = itor->second;
        ver.setUserVerified(user_id);

        result = ver.getUserVerified(user_id_1) && ver.getUserVerified(user_id_2);
    }

    if (result) {
        bool error = false;
        try {
            serverManager.getPrivateRoomId(user_id_1, user_id_2);
            error = true;
        }
        catch (...) {
            serverManager.addPrivateRoom(user_id_1, user_id_2);

            // update the 1st user's friend list
            {
                if (!serverManager.hasUser(user_id_1))
                    throw std::system_error(make_error_code(qls_errc::user_not_existed),
                        "The first user doesn't exist!");
                auto ptr = serverManager.getUser(user_id_1);
                auto set = std::move(ptr->getFriendList());
                set.insert(user_id_2);
                ptr->updateFriendList(std::move(set));

                ptr->removeFriendVerification(user_id_2);
            }
            
            // update the 2nd user's friend list
            {
                if (!serverManager.hasUser(user_id_2))
                    throw std::system_error(make_error_code(qls_errc::user_not_existed),
                        "The second user doesn't exist!");
                auto ptr = serverManager.getUser(user_id_2);
                auto set = std::move(ptr->getFriendList());
                set.insert(user_id_1);
                ptr->updateFriendList(std::move(set));

                ptr->removeFriendVerification(user_id_1);
            }

            // notify the other successfully adding a friend
            qjson::JObject json(qjson::JValueType::JDict);
            json["userid"] = user_id_1.getOriginValue();
            json["type"] = "added_friend";
            auto pack = DataPackage::makePackage(qjson::JWriter::fastWrite(json));
            pack->type = 1;
            serverManager.getUser(user_id_2)->notifyAll(pack->packageToString());
        }

        if (error)
            throw std::system_error(make_error_code(qls_errc::private_room_existed));
    }

    return result;
}

void VerificationManager::removeFriendRoomVerification(UserID user_id_1, UserID user_id_2)
{
    if (user_id_1 == user_id_2)
        throw std::system_error(make_error_code(qls_errc::invalid_verification));

    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_FriendRoomVerification_map_mutex);

        auto itor = m_FriendRoomVerification_map.find({ user_id_1, user_id_2 });
        if (itor == m_FriendRoomVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        m_FriendRoomVerification_map.erase(itor);
    }

    serverManager.getUser(user_id_1)->removeFriendVerification(user_id_2);
    serverManager.getUser(user_id_2)->removeFriendVerification(user_id_1);
}

void VerificationManager::addGroupRoomVerification(GroupID group_id, UserID user_id)
{
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_GroupVerification_map_mutex);

        if (m_GroupVerification_map.find({ group_id, user_id }) !=
            m_GroupVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        m_GroupVerification_map.emplace(GroupVerificationStruct{ group_id, user_id },
                                        GroupRoomVerification{ group_id, user_id });
    }

    // 用户发送请求
    {
        qls::UserVerificationStructure uv;

        uv.user_id = group_id;
        uv.verification_type =
            qls::UserVerificationStructure::VerificationType::Sent;

        auto ptr = serverManager.getUser(user_id);
        ptr->addGroupVerification(group_id, std::move(uv));
    }

    // 群聊拥有者接收请求
    {
        qls::UserVerificationStructure uv;

        uv.user_id = user_id;
        uv.verification_type =
            qls::UserVerificationStructure::VerificationType::Received;

        auto ptr = serverManager.getUser(serverManager.getGroupRoom(group_id)->getAdministrator());
        ptr->addGroupVerification(group_id, std::move(uv));
    }

    // 未完成 通知另一方
    // m_globalRoom->baseSendData()
}

bool VerificationManager::hasGroupRoomVerification(GroupID group_id, UserID user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_GroupVerification_map_mutex);

    return m_GroupVerification_map.find({ group_id, user_id }) !=
        m_GroupVerification_map.cend();
}

bool VerificationManager::setGroupRoomGroupVerified(GroupID group_id, UserID user_id)
{
    bool result = false;
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        auto& ver = itor->second;
        ver.setGroupVerified();
        result = ver.getGroupVerified() && ver.getUserVerified();
    }

    if (result)
    {
        if (!serverManager.hasGroupRoom(group_id))
            throw std::system_error(make_error_code(qls_errc::group_room_not_existed));
        serverManager.getGroupRoom(group_id)->addMember(user_id);

        // 更新user的groupList
        if (!serverManager.hasUser(user_id))
            throw std::system_error(make_error_code(qls_errc::user_not_existed));
        auto ptr = serverManager.getUser(user_id);
        auto set = std::move(ptr->getGroupList());
        set.insert(group_id);
        ptr->updateGroupList(std::move(set));

        // 未完成 通知另一方
        // m_globalRoom->baseSendData()
    }
    return result;
}

bool VerificationManager::setGroupRoomUserVerified(GroupID group_id, UserID user_id)
{
    bool result = false;
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        auto& ver = itor->second;
        ver.setUserVerified();
        result = ver.getGroupVerified() && ver.getUserVerified();
    }

    if (result)
    {
        if (!serverManager.hasGroupRoom(group_id))
            throw std::system_error(make_error_code(qls_errc::group_room_not_existed));
        serverManager.getGroupRoom(group_id)->addMember(user_id);

        // 更新user的friendlist
        if (!serverManager.hasUser(user_id))
            throw std::system_error(make_error_code(qls_errc::user_not_existed));

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

void VerificationManager::removeGroupRoomVerification(GroupID group_id, UserID user_id)
{
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_GroupVerification_map_mutex);

        auto itor = m_GroupVerification_map.find({ group_id, user_id });
        if (itor == m_GroupVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        m_GroupVerification_map.erase(itor);
    }
    serverManager.getUser(serverManager.getGroupRoom(group_id)->getAdministrator())
        ->removeGroupVerification(group_id, user_id);
    serverManager.getUser(user_id)->removeGroupVerification(group_id, user_id);
}

} // namespace qls
