#include "groupRoom.h"

#include <stdexcept>
#include <algorithm>
#include <format>
#include <memory>
#include <system_error>

#include <Json.h>

#include "manager.h"
#include "qls_error.h"
#include "returnStateMessage.hpp"

extern qls::Manager serverManager;

namespace qls
{

struct GroupRoomImpl
{
    long long               m_group_id;
    long long               m_administrator_user_id;
    std::shared_mutex       m_administrator_user_id_mutex;

    std::atomic<bool>       m_can_be_used;

    GroupPermission         m_permission;

    std::unordered_map<long long, GroupRoom::UserDataStructure>
                            m_user_id_map;
    std::shared_mutex       m_user_id_map_mutex;

    std::unordered_map<long long,
        std::pair<std::chrono::time_point<std::chrono::system_clock,
        std::chrono::milliseconds>, std::chrono::minutes>>
                            m_muted_user_map;
    std::shared_mutex       m_muted_user_map_mutex;

    std::vector<std::pair<std::chrono::time_point<std::chrono::system_clock,
        std::chrono::milliseconds>, MessageStructure>>
                            m_message_queue;
    std::shared_mutex       m_message_queue_mutex;
};

// GroupRoom
GroupRoom::GroupRoom(long long group_id, long long administrator, bool is_create):
    m_impl(std::make_unique<GroupRoomImpl>())
{
    m_impl->m_group_id = group_id;
    m_impl->m_administrator_user_id = administrator;

    if (is_create)
    {
        // 创建群聊 sql
        m_impl->m_can_be_used = true;
    }
    else
    {
        // 加载群聊 sql
        m_impl->m_can_be_used = true;
    }
}

GroupRoom::~GroupRoom() = default;

bool GroupRoom::addMember(long long user_id)
{
    if (!m_impl->m_can_be_used)
            throw std::system_error(qls_errc::group_room_unable_to_use);
    std::lock_guard<std::shared_mutex> lg(m_impl->m_user_id_map_mutex);
    if (m_impl->m_user_id_map.find(user_id) == m_impl->m_user_id_map.end())
        m_impl->m_user_id_map[user_id] = UserDataStructure{ serverManager.getUser(user_id)->getUserName(), 1 };

    return true;
}

bool GroupRoom::removeMember(long long user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    std::lock_guard<std::shared_mutex> lg(m_impl->m_user_id_map_mutex);
    if (m_impl->m_user_id_map.find(user_id) != m_impl->m_user_id_map.end())
        m_impl->m_user_id_map.erase(user_id);

    return true;
}

void GroupRoom::sendMessage(long long sender_user_id, std::string_view message)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    // 是否有此user_id
    if (!hasUser(sender_user_id))
        return;

    // 发送者是否被禁言
    {
        std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_muted_user_map_mutex);
        auto itor = m_impl->m_muted_user_map.find(sender_user_id);
        if (itor != m_impl->m_muted_user_map.end())
        {
            if (itor->second.first + itor->second.second >=
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now()))
            {
                return;
            }
            else
            {
                local_shared_lock.unlock();
                std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_muted_user_map_mutex);
                m_impl->m_muted_user_map.erase(sender_user_id);
            }
        }
    }

    // 存储数据
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_message_queue_mutex);
        m_impl->m_message_queue.push_back(
            { std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now()),
                {sender_user_id, std::string(message),
                MessageType::NOMAL_MESSAGE} });
    }

    qjson::JObject json;
    json["type"] = "group_message";
    json["data"]["user_id"] = sender_user_id;
    json["data"]["group_id"] = m_impl->m_group_id;
    json["data"]["message"] = message;

    auto returnJson = qjson::JWriter::fastWrite(json);

    sendData(returnJson);
}

void GroupRoom::sendTipMessage(long long sender_user_id,
    std::string_view message)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    // 是否有此user_id
    if (!hasUser(sender_user_id))
        return;

    // 发送者是否被禁言
    {
        std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_muted_user_map_mutex);
        auto itor = m_impl->m_muted_user_map.find(sender_user_id);
        if (itor != m_impl->m_muted_user_map.end())
        {
            if (itor->second.first + itor->second.second >=
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now()))
            {
                return;
            }
            else
            {
                local_shared_lock.unlock();
                std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_muted_user_map_mutex);
                m_impl->m_muted_user_map.erase(sender_user_id);
            }
        }
    }

    // 存储数据
    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_message_queue_mutex);
        m_impl->m_message_queue.push_back(
            { std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now()),
                {sender_user_id, std::string(message),
                MessageType::TIP_MESSAGE} });
    }

    qjson::JObject json;
    json["type"] = "group_tip_message";
    json["data"]["user_id"] = sender_user_id;
    json["data"]["group_id"] = m_impl->m_group_id;
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json));
}

void GroupRoom::sendUserTipMessage(long long sender_user_id,
    std::string_view message, long long receiver_user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    // 是否有此user_id
    if (!hasUser(receiver_user_id))
        return;

    // 发送者是否被禁言
    {
        std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_muted_user_map_mutex);
        auto itor = m_impl->m_muted_user_map.find(sender_user_id);
        if (itor != m_impl->m_muted_user_map.end())
        {
            if (itor->second.first + itor->second.second >=
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now()))
            {
                return;
            }
            else
            {
                local_shared_lock.unlock();
                std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_muted_user_map_mutex);
                m_impl->m_muted_user_map.erase(sender_user_id);
            }
        }
    }

    qjson::JObject json;
    json["type"] = "group_tip_message";
    json["data"]["user_id"] = sender_user_id;
    json["data"]["group_id"] = m_impl->m_group_id;
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json), receiver_user_id);
}

void GroupRoom::getMessage(
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    if (from > to)
    {
        sendData(qjson::JWriter::fastWrite(makeErrorMessage("Illegal time point")));
        return;
    }

    auto searchPoint = [this](
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& p,
        bool edge = false) -> size_t {
        
        size_t left = 0ull;
        size_t right = m_impl->m_message_queue.size() - 1;
        size_t middle = (left + right) / 2;

        while (left < right - 1)
        {
            if (m_impl->m_message_queue[middle].first.time_since_epoch().count() ==
                p.time_since_epoch().count())
            {
                return middle;
            }
            else if (m_impl->m_message_queue[middle].first.time_since_epoch().count() <
                p.time_since_epoch().count())
            {
                left = middle;
                middle = (left + right) / 2;
            }
            else
            {
                right = middle;
                middle = (left + right) / 2;
            }
        }

        return edge ? left : right;
        };

    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_message_queue_mutex);
    if (m_impl->m_message_queue.empty())
    {
        sendData(qjson::JWriter::fastWrite(qjson::JObject(qjson::JValueType::JList)));
        return;
    }

    std::sort(m_impl->m_message_queue.begin(), m_impl->m_message_queue.end(), [](
        const std::pair<std::chrono::system_clock::time_point, MessageStructure>& a,
        const std::pair<std::chrono::system_clock::time_point, MessageStructure>& b)
        {return a.first.time_since_epoch().count() < b.first.time_since_epoch().count();});

    size_t from_itor = searchPoint(from, true);
    size_t to_itor = searchPoint(to, false);

    qjson::JObject returnJson(qjson::JValueType::JList);
    for (auto i = from_itor; i <= to_itor; i++)
    {
        switch (m_impl->m_message_queue[i].second.type)
        {
        case MessageType::NOMAL_MESSAGE:
        {
            const auto& MessageStructure = m_impl->m_message_queue[i].second;
            qjson::JObject localjson;
            localjson["type"] = "group_message";
            localjson["data"]["user_id"] = MessageStructure.user_id;
            localjson["data"]["group_id"] = m_impl->m_group_id;
            localjson["data"]["message"] = MessageStructure.message;
            localjson["time_point"] = m_impl->m_message_queue[i].first.time_since_epoch().count();
            returnJson.push_back(std::move(localjson));
        }
            break;
        case MessageType::TIP_MESSAGE:
        {
            const auto& MessageStructure = m_impl->m_message_queue[i].second;
            qjson::JObject localjson;
            localjson["type"] = "group_tip_message";
            localjson["data"]["user_id"] = MessageStructure.user_id;
            localjson["data"]["group_id"] = m_impl->m_group_id;
            localjson["data"]["message"] = MessageStructure.message;
            localjson["time_point"] = m_impl->m_message_queue[i].first.time_since_epoch().count();
            returnJson.push_back(std::move(localjson));
        }
            break;
        default:
            break;
        }
    }

    sendData(qjson::JWriter::fastWrite(returnJson));
}

bool GroupRoom::hasUser(long long user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex);
    return m_impl->m_user_id_map.find(user_id) != m_impl->m_user_id_map.end();
}

std::unordered_map<long long, GroupRoom::UserDataStructure> GroupRoom::getUserList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex);
    return m_impl->m_user_id_map;
}

std::string GroupRoom::getUserNickname(long long user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex);
    auto itor = m_impl->m_user_id_map.find(user_id);
    if (itor == m_impl->m_user_id_map.end())
        throw std::system_error(qls_errc::user_not_existed, "user isn't in the room");

    return itor->second.nickname;
}

long long GroupRoom::getUserGroupLevel(long long user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex);
    auto itor = m_impl->m_user_id_map.find(user_id);
    if (itor == m_impl->m_user_id_map.end())
        throw std::system_error(qls_errc::user_not_existed, "user isn't in the room");

    return itor->second.groupLevel;
}

std::unordered_map<long long,
    PermissionType> GroupRoom::getUserPermissionList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    return std::move(m_impl->m_permission.getUserPermissionList());
}

long long GroupRoom::getAdministrator() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_administrator_user_id_mutex);
    return m_impl->m_administrator_user_id;
}

void GroupRoom::setAdministrator(long long user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);

    std::unique_lock<std::shared_mutex> local_unique_lock1(m_impl->m_user_id_map_mutex, std::defer_lock);
    std::unique_lock<std::shared_mutex> local_unique_lock2(m_impl->m_administrator_user_id_mutex, std::defer_lock);
    std::lock(local_unique_lock1, local_unique_lock2);

    if (m_impl->m_administrator_user_id == 0)
    {
        auto itor = m_impl->m_user_id_map.find(user_id);
        if (itor == m_impl->m_user_id_map.end())
        {
            m_impl->m_user_id_map[user_id] = UserDataStructure{
                serverManager.getUser(user_id)->getUserName(), 1 };
            m_impl->m_permission.modifyUserPermission(user_id,
                PermissionType::Administrator);
        }
        else
        {
            m_impl->m_permission.modifyUserPermission(user_id,
                PermissionType::Administrator);
        }
    }
    else
    {
        m_impl->m_permission.modifyUserPermission(m_impl->m_administrator_user_id,
            PermissionType::Default);
        m_impl->m_permission.modifyUserPermission(user_id,
            PermissionType::Administrator);
        m_impl->m_administrator_user_id = user_id;
    }
}

long long GroupRoom::getGroupID() const
{
    return m_impl->m_group_id;
}

std::vector<long long> GroupRoom::getDefaultUserList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    return std::move(m_impl->m_permission.getDefaultUserList());
}

std::vector<long long> GroupRoom::getOperatorList() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    return std::move(m_impl->m_permission.getOperatorList());
}

bool GroupRoom::muteUser(long long executorId, long long user_id, const std::chrono::minutes& mins)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    if (executorId == user_id || !hasUser(user_id) || !hasUser(executorId))
        return false;

    auto executorIdType = m_impl->m_permission.getUserPermissionType(executorId);
    auto userIdType = m_impl->m_permission.getUserPermissionType(user_id);
    if (userIdType >= executorIdType)
        return false;

    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_muted_user_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex, std::defer_lock);
    std::lock(local_unique_lock, local_shared_lock);

    m_impl->m_muted_user_map[user_id] = std::pair<std::chrono::time_point<std::chrono::system_clock,
        std::chrono::milliseconds>,
        std::chrono::minutes>{ std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now()),
        mins };
    sendTipMessage(executorId, std::format("{} was muted by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executorId].nickname));

    return true;
}

bool GroupRoom::unmuteUser(long long executorId, long long user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    if (executorId == user_id || !hasUser(user_id) || !hasUser(executorId))
        return false;

    auto executorIdType = m_impl->m_permission.getUserPermissionType(executorId);
    auto userIdType = m_impl->m_permission.getUserPermissionType(user_id);
    if (userIdType >= executorIdType)
        return false;

    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_muted_user_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex, std::defer_lock);
    std::lock(local_unique_lock, local_shared_lock);

    m_impl->m_muted_user_map.erase(user_id);
    sendTipMessage(executorId, std::format("{} was unmuted by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executorId].nickname));

    return true;
}

bool GroupRoom::kickUser(long long executorId, long long user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    if (executorId == user_id || !hasUser(user_id) || !hasUser(executorId))
        return false;

    auto executorIdType = m_impl->m_permission.getUserPermissionType(executorId);
    auto userIdType = m_impl->m_permission.getUserPermissionType(user_id);
    if (userIdType >= executorIdType)
        return false;

    std::unique_lock<std::shared_mutex> local_unique_lock1(m_impl->m_user_id_map_mutex, std::defer_lock),
        local_shared_lock2(m_impl->m_muted_user_map_mutex, std::defer_lock);
    std::lock(local_unique_lock1, local_shared_lock2);
    sendTipMessage(executorId, std::format("{} was kicked by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executorId].nickname));
    m_impl->m_user_id_map.erase(user_id);

    return true;
}

bool GroupRoom::addOperator(long long executorId, long long user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    if (executorId == user_id || !hasUser(user_id) || !hasUser(executorId))
        return false;

    if (m_impl->m_permission.getUserPermissionType(executorId) !=
        PermissionType::Administrator)
        return false;
    if (m_impl->m_permission.getUserPermissionType(user_id) !=
        PermissionType::Default)
        return false;

    m_impl->m_permission.modifyUserPermission(user_id,
        PermissionType::Operator);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex);
    sendTipMessage(executorId, std::format("{} was turned operator by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executorId].nickname));

    return true;
}

bool GroupRoom::removeOperator(long long executorId, long long user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(qls_errc::group_room_unable_to_use);
    if (executorId == user_id || !hasUser(user_id) || !hasUser(executorId))
        return false;

    if (m_impl->m_permission.getUserPermissionType(executorId) !=
        PermissionType::Administrator)
        return false;
    if (m_impl->m_permission.getUserPermissionType(user_id) !=
        PermissionType::Operator)
        return false;

    m_impl->m_permission.modifyUserPermission(user_id,
        PermissionType::Default);

    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_id_map_mutex);
    sendTipMessage(executorId, std::format("{} was turned default user by {}",
        m_impl->m_user_id_map[user_id].nickname, m_impl->m_user_id_map[executorId].nickname));

    return true;
}

void GroupRoom::removeThisRoom()
{
    m_impl->m_can_be_used = false;

    {
        // sql 上面删除此房间
    }

    // 剩下其他东西
}

bool GroupRoom::canBeUsed() const
{
    return m_impl->m_can_be_used;
}

} // namespace qls
