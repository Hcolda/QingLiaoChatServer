#include "verificationManager.h"

#include <mutex>
#include <shared_mutex>
#include <functional>
#include <utility>
#include <Json.h>
#include <dataPackage.h>

#include "user.h"
#include "groupid.hpp"
#include "userid.hpp"
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
    if (!serverManager.hasUser(user_id_1))
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "the id of user1 is invalid");
    if (!serverManager.hasUser(user_id_2))
        throw std::system_error(make_error_code(qls_errc::user_not_existed), "the id of user2 is invalid");

    // check if they are friends
    if (serverManager.hasPrivateRoom(user_id_1, user_id_2))
        throw std::system_error(make_error_code(qls_errc::private_room_existed));

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
        qls::Verification::UserVerification uv;

        uv.user_id = user_id_2;
        uv.verification_type =
            qls::Verification::VerificationType::Sent;

        auto ptr = serverManager.getUser(user_id_1);
        ptr->addFriendVerification(user_id_2, std::move(uv));
    }

    // user2
    {
        qls::Verification::UserVerification uv;

        uv.user_id = user_id_1;
        uv.verification_type =
            qls::Verification::VerificationType::Received;

        auto ptr = serverManager.getUser(user_id_2);
        ptr->addFriendVerification(user_id_1, std::move(uv));

        // notify the other successfully adding a friend
        qjson::JObject json(qjson::JValueType::JDict);
        json["userid"] = user_id_1.getOriginValue();
        json["type"] = "added_friend_verfication";
        json["message"] = "";
        sendToUser(user_id_2, json);
    }
}

bool VerificationManager::hasFriendRoomVerification(UserID user_id_1, UserID user_id_2) const
{
    if (user_id_1 == user_id_2)
        return false;

    std::shared_lock<std::shared_mutex> local_shared_lock(m_FriendRoomVerification_map_mutex);
    return m_FriendRoomVerification_map.find({ user_id_1, user_id_2 }) !=
        m_FriendRoomVerification_map.cend();
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
        if (itor == m_FriendRoomVerification_map.cend())
            throw std::system_error(make_error_code(qls_errc::verification_not_existed));

        auto& ver = itor->second;
        ver.setUserVerified(user_id);

        result = ver.getUserVerified(user_id_1) && ver.getUserVerified(user_id_2);
        if (result) {
            m_FriendRoomVerification_map.erase(itor);
        }
    }

    if (result) {
        serverManager.addPrivateRoom(user_id_1, user_id_2);

        // update the 1st user's friend list
        {
            auto ptr = serverManager.getUser(user_id_1);
            ptr->updateFriendList([user_id_2](std::unordered_set<qls::UserID>& set){
                set.insert(user_id_2);
            });

            ptr->removeFriendVerification(user_id_2);
        }
        
        // update the 2nd user's friend list
        {
            auto ptr = serverManager.getUser(user_id_2);
            ptr->updateFriendList([user_id_1](std::unordered_set<qls::UserID>& set){
                set.insert(user_id_1);
            });

            ptr->removeFriendVerification(user_id_1);
        }

        // notify the other successfully adding a friend
        qjson::JObject json(qjson::JValueType::JDict);
        json["userid"] = user_id_1.getOriginValue();
        json["type"] = "added_friend";
        sendToUser(user_id_2, json);
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

    // notify them to remove the friend verification
    // (someone reject to add a friend)
    {
        // user1
        qjson::JObject json(qjson::JValueType::JDict);
        json["userid"] = user_id_2.getOriginValue();
        json["type"] = "rejected_to_add_friend";
        sendToUser(user_id_1, json);
    }
    {
        // user2
        qjson::JObject json(qjson::JValueType::JDict);
        json["userid"] = user_id_1.getOriginValue();
        json["type"] = "rejected_to_add_friend";
        sendToUser(user_id_2, json);
    }
}

void VerificationManager::addGroupRoomVerification(GroupID group_id, UserID user_id)
{
    if (!serverManager.hasGroupRoom(group_id))
            throw std::system_error(make_error_code(qls_errc::group_room_not_existed));
    if (!serverManager.hasUser(user_id))
            throw std::system_error(make_error_code(qls_errc::user_not_existed));

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
        qls::Verification::GroupVerification uv;

        uv.group_id = group_id;
        uv.user_id = user_id;
        uv.verification_type = qls::Verification::Sent;

        auto ptr = serverManager.getUser(user_id);
        ptr->addGroupVerification(group_id, std::move(uv));
    }

    // 群聊拥有者接收请求
    {
        qls::Verification::GroupVerification uv;

        uv.group_id = group_id;
        uv.user_id = user_id;
        uv.verification_type = qls::Verification::Received;

        UserID adminID = serverManager.getGroupRoom(group_id)->getAdministrator();
        auto ptr = serverManager.getUser(adminID);
        ptr->addGroupVerification(group_id, std::move(uv));

        qjson::JObject json(qjson::JValueType::JDict);
        json["groupid"] = group_id.getOriginValue();
        json["userid"] = user_id.getOriginValue();
        json["type"] = "added_group_verification";
        json["message"] = "";
        sendToUser(adminID, json);
    }
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
        serverManager.getGroupRoom(group_id)->addMember(user_id);

        // 更新user的groupList
        auto ptr = serverManager.getUser(user_id);
        ptr->updateGroupList([group_id](std::unordered_set<qls::GroupID>& set){
            set.insert(group_id);
        });

        // notify the other successfully adding a group
        qjson::JObject json(qjson::JValueType::JDict);
        json["groupid"] = group_id.getOriginValue();
        json["type"] = "added_group";
        sendToUser(user_id, json);
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
        serverManager.getGroupRoom(group_id)->addMember(user_id);

        // 更新user的friendlist
        {
            auto ptr = serverManager.getUser(user_id);
            ptr->updateGroupList([group_id](std::unordered_set<qls::GroupID>& set){
                set.insert(group_id);
            });

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
    UserID adminID = serverManager.getGroupRoom(group_id)->getAdministrator();
    serverManager.getUser(adminID)
        ->removeGroupVerification(group_id, user_id);
    serverManager.getUser(user_id)->removeGroupVerification(group_id, user_id);

    {
        // user
        qjson::JObject json(qjson::JValueType::JDict);
        json["groupid"] = group_id.getOriginValue();
        json["type"] = "rejected_to_add_group";
        sendToUser(user_id, json);
    }
    {
        // group admin
        qjson::JObject json(qjson::JValueType::JDict);
        json["groupid"] = group_id.getOriginValue();
        json["userid"] = user_id.getOriginValue();
        json["type"] = "rejected_to_add_member_to_group";
        json["message"] = "";
        sendToUser(adminID, json);
    }
}

void VerificationManager::sendToUser(qls::UserID user_id, const qjson::JObject& json)
{
    auto pack = DataPackage::makePackage(qjson::JWriter::fastWrite(json));
    pack->type = DataPackage::Text;
    serverManager.getUser(user_id)->notifyAll(pack->packageToString());
}

} // namespace qls
