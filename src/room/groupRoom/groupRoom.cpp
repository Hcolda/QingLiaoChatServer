#include "groupRoom.h"

#include <stdexcept>

#include <QuqiCrypto.hpp>
#include <Json.h>

#include "manager.h"

extern qls::Manager serverManager;

namespace qls
{
    // GroupRoom

    GroupRoom::GroupRoom(long long group_id) :
        m_group_id(group_id),
        m_administrator_user_id(0)
    {
        // group room 各项权限
    }

    void GroupRoom::init()
    {
    }

    bool GroupRoom::addMember(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) == m_user_id_map.end())
            m_user_id_map[user_id] = UserDataStruct{ serverManager.getUser(user_id)->getUserName(), 1 };

        return true;
    }

    bool GroupRoom::removeMember(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) != m_user_id_map.end())
            m_user_id_map.erase(user_id);

        return true;
    }

    bool GroupRoom::joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user)
    {
        return baseJoinRoom(socket_ptr, user);
    }

    bool GroupRoom::leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        return baseLeaveRoom(socket_ptr);
    }

    asio::awaitable<bool> GroupRoom::sendMessage(long long sender_user_id, const std::string& message)
    {
        // 是否有此user_id
        if (!hasUser(sender_user_id))
            co_return false;

        // 存储数据
        {
            std::unique_lock<std::shared_mutex> ul(m_message_queue_mutex);
            this->m_message_queue.push_back(
                { std::chrono::system_clock::now(),
                    {sender_user_id, message,
                    MessageStruct::MessageType::NOMAL_MESSAGE} });
        }

        qjson::JObject json;
        json["type"] = "group_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        auto rejson = qjson::JWriter::fastWrite(json);

        co_return co_await baseSendData(rejson);
    }

    asio::awaitable<bool> GroupRoom::sendTipMessage(long long sender_user_id, const std::string& message)
    {
        // 存储数据
        {
            std::unique_lock<std::shared_mutex> ul(m_message_queue_mutex);
            this->m_message_queue.push_back(
                { std::chrono::system_clock::now(),
                    {sender_user_id, message,
                    MessageStruct::MessageType::TIP_MESSAGE} });
        }

        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> GroupRoom::sendUserTipMessage(long long sender_user_id, const std::string& message, long long receiver_user_id)
    {
        // 是否有此user_id
        if (!hasUser(receiver_user_id))
            co_return false;

        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json), receiver_user_id);
    }

    bool GroupRoom::hasUser(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
        return m_user_id_map.find(user_id) != m_user_id_map.end();
    }

    long long GroupRoom::getAdministrator() const
    {
        std::shared_lock<std::shared_mutex> sl(m_administrator_user_id_mutex);
        return m_administrator_user_id;
    }

    void GroupRoom::setAdministrator(long long user_id)
    {
        std::unique_lock<std::shared_mutex> ul1(m_user_id_map_mutex, std::defer_lock);
        std::unique_lock<std::shared_mutex> ul2(m_administrator_user_id_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        if (m_administrator_user_id == 0)
        {
            auto itor = m_user_id_map.find(user_id);
            if (itor == m_user_id_map.end())
            {
                m_user_id_map[user_id] = UserDataStruct{ serverManager.getUser(user_id)->getUserName(), 1 };
                m_permission.modifyUserPermission(user_id,
                    GroupPermission::PermissionType::Administrator);
            }
            else
            {
                m_permission.modifyUserPermission(user_id,
                    GroupPermission::PermissionType::Administrator);
            }
        }
        else
        {
            m_permission.modifyUserPermission(m_administrator_user_id,
                GroupPermission::PermissionType::Default);
            m_permission.modifyUserPermission(user_id,
                GroupPermission::PermissionType::Administrator);
            m_administrator_user_id = user_id;
        }
    }

    long long GroupRoom::getGroupID() const
    {
        return m_group_id;
    }

    bool GroupRoom::muteUser(long long executorId, long long user_id, const std::chrono::minutes& mins)
    {
        if (executorId == user_id &&
            !this->hasUser(user_id) &&
            !this->hasUser(executorId))
            return false;

        auto executorIdType = this->m_permission.getUserPermissionType(executorId);
        auto userIdType = this->m_permission.getUserPermissionType(user_id);
        if (userIdType >= executorIdType)
            return false;

        std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
        m_muted_user_map[user_id] = std::pair<std::chrono::system_clock::time_point,
            std::chrono::minutes>{ std::chrono::system_clock::now(), mins };
        ul.unlock();

        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        this->sendTipMessage(executorId, std::format("{} was muted by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname));
        sl.unlock();

        return true;
    }

    bool GroupRoom::unmuteUser(long long executorId, long long user_id)
    {
        if (executorId == user_id &&
            !this->hasUser(user_id) &&
            !this->hasUser(executorId))
            return false;

        auto executorIdType = this->m_permission.getUserPermissionType(executorId);
        auto userIdType = this->m_permission.getUserPermissionType(user_id);
        if (userIdType >= executorIdType)
            return false;

        std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
        m_muted_user_map.erase(user_id);
        ul.unlock();

        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        this->sendTipMessage(executorId, std::format("{} was unmuted by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname));
        sl.unlock();

        return true;
    }

    bool GroupRoom::kickUser(long long executorId, long long user_id)
    {
        if (executorId == user_id &&
            !this->hasUser(user_id) &&
            !this->hasUser(executorId))
            return false;

        auto executorIdType = this->m_permission.getUserPermissionType(executorId);
        auto userIdType = this->m_permission.getUserPermissionType(user_id);
        if (userIdType >= executorIdType)
            return false;

        std::unique_lock<std::shared_mutex> sl1(m_user_id_map_mutex, std::defer_lock),
            sl2(m_muted_user_map_mutex, std::defer_lock);

        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        this->sendTipMessage(executorId, std::format("{} was kicked by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname));
        sl.unlock();

        return true;
    }

    bool GroupRoom::addOperator(long long executorId, long long user_id)
    {
        if (!this->hasUser(user_id) && !this->hasUser(executorId))
            return false;

        if (this->m_permission.getUserPermissionType(executorId) !=
            GroupPermission::PermissionType::Administrator)
            return false;

        this->m_permission.modifyUserPermission(user_id,
            GroupPermission::PermissionType::Operator);

        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        this->sendTipMessage(executorId, std::format("{} was turned operator by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname));
        sl.unlock();

        return true;
    }

    bool GroupRoom::removeOperator(long long executorId, long long user_id)
    {
        if (executorId == user_id &&
            !this->hasUser(user_id) &&
            !this->hasUser(executorId))
            return false;

        if (this->m_permission.getUserPermissionType(executorId) !=
            GroupPermission::PermissionType::Administrator)
            return false;

        this->m_permission.modifyUserPermission(user_id,
            GroupPermission::PermissionType::Default);

        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        this->sendTipMessage(executorId, std::format("{} was turned default user by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname));
        sl.unlock();

        return true;
    }

    void GroupRoom::GroupPermission::modifyPermission(const std::string& permissionName, PermissionType type)
    {
        std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);
        m_permission_map[permissionName] = type;
    }

    void GroupRoom::GroupPermission::removePermission(const std::string& permissionName)
    {
        std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);

        // 是否有此权限
        auto itor = m_permission_map.find(permissionName);
        if (itor == m_permission_map.end())
            throw std::invalid_argument("No permission: " + permissionName);

        m_permission_map.erase(itor);
    }

    GroupRoom::GroupPermission::PermissionType GroupRoom::GroupPermission::getPermissionType(const std::string& permissionName) const
    {
        std::shared_lock<std::shared_mutex> sl(m_permission_map_mutex);

        // 是否有此权限
        auto itor = m_permission_map.find(permissionName);
        if (itor == m_permission_map.end())
            throw std::invalid_argument("No permission: " + permissionName);

        return itor->second;
    }

    void GroupRoom::GroupPermission::modifyUserPermission(long long user_id, PermissionType type)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);
        m_user_permission_map[user_id] = type;
    }

    void GroupRoom::GroupPermission::removeUser(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::invalid_argument("No user: " + std::to_string(user_id));

        m_user_permission_map.erase(itor);
    }

    bool GroupRoom::GroupPermission::userHasPermission(long long user_id, const std::string& permissionName) const
    {
        std::shared_lock<std::shared_mutex> sl1(m_permission_map_mutex, std::defer_lock);
        std::shared_lock<std::shared_mutex> sl2(m_user_permission_map_mutex, std::defer_lock);
        // 同时加锁
        std::lock(sl1, sl2);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::invalid_argument("No user: " + std::to_string(user_id));

        // 是否有此权限
        auto itor2 = m_permission_map.find(permissionName);
        if (itor2 == m_permission_map.end())
            throw std::invalid_argument("No permission: " + permissionName);

        // 返回权限
        return itor->second >= itor2->second;
    }

    GroupRoom::GroupPermission::PermissionType GroupRoom::GroupPermission::getUserPermissionType(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::invalid_argument("No user: " + std::to_string(user_id));

        return itor->second;
    }
}