#include "room.h"

#include <QuqiCrypto.hpp>
#include <Json.h>

namespace qls
{
    bool BaseRoom::baseJoinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user)
    {
        if (socket_ptr.get() == nullptr) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        m_userMap[socket_ptr] = user;
        return true;
    }

    bool BaseRoom::baseLeaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        if (socket_ptr.get() == nullptr) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        if (m_userMap.find(socket_ptr) == m_userMap.end()) return false;
            m_userMap.erase(m_userMap.find(socket_ptr));

        return true;
    }

    asio::awaitable<bool> BaseRoom::baseSendData(const std::string& data)
    {
        // 广播数据
        {
            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                try
                {
                    std::string out;
                    if (!aes.encrypt(data, out, { i->second.key, 32 }, { i->second.iv, 16 }, true))
                        throw std::logic_error("Key and ivec of AES are invalid");
                    co_await i->first->async_send(asio::buffer(out), asio::use_awaitable);
                }
                catch (...)
                {
                    std::lock_guard<std::shared_mutex> lock(m_userDeleteQueue_mutex);
                    m_userDeleteQueue.push(i->first);
                }
            }
        }

        // 去除已经关闭的连接
        {
            std::lock_guard<std::shared_mutex> lock(m_userDeleteQueue_mutex);
            if (!m_userDeleteQueue.empty())
            {
                do
                {
                    std::shared_ptr<asio::ip::tcp::socket> localSocket_ptr = std::move(m_userDeleteQueue.front());
                    auto itor = m_userMap.find(localSocket_ptr);
                    if (itor != m_userMap.end())
                    {
                        m_userMap.erase(itor);
                    }

                    m_userDeleteQueue.pop();
                } while (!m_userDeleteQueue.empty());
            }
        }

        co_return true;
    }

    asio::awaitable<bool> BaseRoom::baseSendData(const std::string& data, long long user_id)
    {
        // 广播数据
        {
            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                if (i->second.user_id == user_id)
                {
                    try
                    {
                        std::string out;
                        if (!aes.encrypt(data, out, { i->second.key, 32 }, { i->second.iv, 16 }, true))
                            throw std::logic_error("Key and ivec of AES are invalid");
                        co_await i->first->async_send(asio::buffer(out), asio::use_awaitable);
                    }
                    catch (...)
                    {
                        std::lock_guard<std::shared_mutex> lockguard(m_userDeleteQueue_mutex);
                        m_userDeleteQueue.push(i->first);
                    }
                }
            }
        }

        // 去除已经关闭的连接
        {
            std::lock_guard<std::shared_mutex> lock(m_userDeleteQueue_mutex);
            if (!m_userDeleteQueue.empty())
            {
                do
                {
                    std::shared_ptr<asio::ip::tcp::socket> localSocket_ptr = std::move(m_userDeleteQueue.front());
                    auto itor = m_userMap.find(localSocket_ptr);
                    if (itor != m_userMap.end())
                    {
                        m_userMap.erase(itor);
                    }

                    m_userDeleteQueue.pop();
                } while (!m_userDeleteQueue.empty());
            }
        }

        co_return true;
    }

    // PrivateRoom

    PrivateRoom::PrivateRoom(long long user_id_1, long long user_id_2) :
        m_user_id_1(user_id_1),
        m_user_id_2(user_id_2)
    {}

    void PrivateRoom::init()
    {
    }

    bool PrivateRoom::joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user)
    {
        if (!hasUser(user.user_id))
            return false;
        return baseJoinRoom(socket_ptr, user);
    }

    bool PrivateRoom::leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        return baseLeaveRoom(socket_ptr);
    }

    asio::awaitable<bool> PrivateRoom::sendMessage(const std::string& message, long long sender_user_id)
    {
        if (!hasUser(sender_user_id))
            co_return false;

        qjson::JObject json;
        json["type"] = "private_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> PrivateRoom::sendTipMessage(const std::string& message)
    {
        qjson::JObject json;
        json["type"] = "private_tip_message";
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    long long PrivateRoom::getUserID1() const
    {
        return m_user_id_1;
    }

    long long PrivateRoom::getUserID2() const
    {
        return m_user_id_1;
    }

    bool PrivateRoom::hasUser(long long user_id) const
    {
        return user_id == m_user_id_1 || user_id == m_user_id_2;
    }

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

    asio::awaitable<bool> GroupRoom::sendMessage(const std::string& message, long long sender_user_id)
    {
        // 是否有此user_id
        if (!hasUser(sender_user_id))
            co_return false;

        qjson::JObject json;
        json["type"] = "group_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> GroupRoom::sendTipMessage(const std::string& message)
    {
        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["group_id"] = m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> GroupRoom::sendUserTipMessage(const std::string& message, long long receiver_user_id)
    {
        // 是否有此user_id
        if (!hasUser(receiver_user_id))
            co_return false;

        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["group_id"] = m_group_id;
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
