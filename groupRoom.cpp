#include "groupRoom.h"

#include <stdexcept>

#include <QuqiCrypto.hpp>
#include <Json.h>

namespace qls
{
    // GroupRoom

    GroupRoom::GroupRoom(long long group_id) :
        m_group_id(group_id) {}

    void GroupRoom::init()
    {
    }

    bool GroupRoom::addMember(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) == m_user_id_map.end())
            m_user_id_map.insert(user_id);

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

        qjson::JObject json;
        json["type"] = "group_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        auto rejson = qjson::JWriter::fastWrite(json);

        // 存储数据
        {
            std::unique_lock<std::shared_mutex> ul(m_message_queue_mutex);
            m_message_queue.push({
                std::chrono::system_clock::now().time_since_epoch().count(), rejson });
        }

        co_return co_await baseSendData(rejson);
    }

    asio::awaitable<bool> GroupRoom::sendTipMessage(long long sender_user_id, const std::string& message)
    {
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

    long long GroupRoom::getGroupID() const
    {
        return m_group_id;
    }

    void GroupRoom::Permission::modifyPermission(const std::string& permissionName, PermissionType type)
    {
        std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);
        m_permission_map[permissionName] = type;
    }

    void GroupRoom::Permission::removePermission(const std::string& permissionName)
    {
        std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);

        // 是否有此权限
        auto itor = m_permission_map.find(permissionName);
        if (itor == m_permission_map.end())
            throw std::invalid_argument("No permission: " + permissionName);

        m_permission_map.erase(itor);
    }

    GroupRoom::Permission::PermissionType GroupRoom::Permission::getPermissionType(const std::string& permissionName) const
    {
        std::shared_lock<std::shared_mutex> sl(m_permission_map_mutex);

        // 是否有此权限
        auto itor = m_permission_map.find(permissionName);
        if (itor == m_permission_map.end())
            throw std::invalid_argument("No permission: " + permissionName);

        return itor->second;
    }

    void GroupRoom::Permission::modifyUserPermission(long long user_id, PermissionType type)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);
        m_user_permission_map[user_id] = type;
    }

    void GroupRoom::Permission::removeUser(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::invalid_argument("No user: " + std::to_string(user_id));

        m_user_permission_map.erase(itor);
    }

    bool GroupRoom::Permission::userHasPermission(long long user_id, const std::string& permissionName) const
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

    GroupRoom::Permission::PermissionType GroupRoom::Permission::getUserPermissionType(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::invalid_argument("No user: " + std::to_string(user_id));

        return itor->second;
    }
}