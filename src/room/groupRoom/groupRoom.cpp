#include "groupRoom.h"

#include <stdexcept>
#include <algorithm>
#include <format>
#include <memory>

#include <Json.h>

#include "manager.h"

extern qls::Manager serverManager;

namespace qls
{
    // GroupRoom
    GroupRoom::GroupRoom(long long group_id, long long administrator, bool is_create) :
        m_group_id(group_id),
        m_administrator_user_id(administrator)
    {
        if (is_create)
        {
            // 创建群聊 sql
            m_can_be_used = true;
        }
        else
        {
            // 加载群聊 sql
            m_can_be_used = true;
        }
    }

    bool GroupRoom::addMember(long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) == m_user_id_map.end())
            m_user_id_map[user_id] = UserDataStruct{ serverManager.getUser(user_id)->getUserName(), 1 };

        return true;
    }

    bool GroupRoom::removeMember(long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) != m_user_id_map.end())
            m_user_id_map.erase(user_id);

        return true;
    }

    asio::awaitable<void> GroupRoom::sendMessage(long long sender_user_id, const std::string& message)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        // 是否有此user_id
        if (!hasUser(sender_user_id))
            co_return;

        // 发送者是否被禁言
        {
            std::shared_lock<std::shared_mutex> sl(m_muted_user_map_mutex);
            auto itor = m_muted_user_map.find(sender_user_id);
            if (itor != m_muted_user_map.end())
            {
                if (itor->second.first + itor->second.second >=
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()))
                {
                    co_return;
                }
                else
                {
                    sl.unlock();
                    std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
                    m_muted_user_map.erase(sender_user_id);
                }
            }
        }

        // 存储数据
        {
            std::unique_lock<std::shared_mutex> ul(m_message_queue_mutex);
            this->m_message_queue.push_back(
                { std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now()),
                    {sender_user_id, message,
                    MessageStruct::MessageType::NOMAL_MESSAGE} });
        }

        qjson::JObject json;
        json["type"] = "group_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        auto returnJson = qjson::JWriter::fastWrite(json);

        sendData(returnJson);
        co_return;
    }

    asio::awaitable<void> GroupRoom::sendTipMessage(long long sender_user_id,
        const std::string& message)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        // 是否有此user_id
        if (!hasUser(sender_user_id))
            co_return;

        // 发送者是否被禁言
        {
            std::shared_lock<std::shared_mutex> sl(m_muted_user_map_mutex);
            auto itor = m_muted_user_map.find(sender_user_id);
            if (itor != m_muted_user_map.end())
            {
                if (itor->second.first + itor->second.second >=
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()))
                {
                    co_return;
                }
                else
                {
                    sl.unlock();
                    std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
                    m_muted_user_map.erase(sender_user_id);
                }
            }
        }

        // 存储数据
        {
            std::unique_lock<std::shared_mutex> ul(m_message_queue_mutex);
            this->m_message_queue.push_back(
                { std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now()),
                    {sender_user_id, message,
                    MessageStruct::MessageType::TIP_MESSAGE} });
        }

        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        sendData(qjson::JWriter::fastWrite(json));
        co_return;
    }

    asio::awaitable<void> GroupRoom::sendUserTipMessage(long long sender_user_id,
        const std::string& message, long long receiver_user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        // 是否有此user_id
        if (!hasUser(receiver_user_id))
            co_return;

        // 发送者是否被禁言
        {
            std::shared_lock<std::shared_mutex> sl(m_muted_user_map_mutex);
            auto itor = m_muted_user_map.find(sender_user_id);
            if (itor != m_muted_user_map.end())
            {
                if (itor->second.first + itor->second.second >=
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()))
                {
                    co_return;
                }
                else
                {
                    sl.unlock();
                    std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
                    m_muted_user_map.erase(sender_user_id);
                }
            }
        }

        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = this->m_group_id;
        json["data"]["message"] = message;

        sendData(qjson::JWriter::fastWrite(json), receiver_user_id);
        co_return;
    }

    asio::awaitable<void> GroupRoom::getMessage(
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (from > to) co_return;

        auto searchPoint = [this](
            const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& p,
            bool edge = false) -> size_t {
            
            size_t left = 0ull;
            size_t right = m_message_queue.size() - 1;
            size_t middle = (left + right) / 2;

            while (left < right - 1)
            {
                if (m_message_queue[middle].first.time_since_epoch().count() ==
                    p.time_since_epoch().count())
                {
                    return middle;
                }
                else if (m_message_queue[middle].first.time_since_epoch().count() <
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

        std::unique_lock<std::shared_mutex> sl(m_message_queue_mutex);
        if (m_message_queue.empty())
        {
            sendData(qjson::JWriter::fastWrite(qjson::JObject(qjson::JValueType::JList)));
            co_return;
        }

        std::sort(m_message_queue.begin(), m_message_queue.end(), [](
            const std::pair<std::chrono::system_clock::time_point, MessageStruct>& a,
            const std::pair<std::chrono::system_clock::time_point, MessageStruct>& b)
            {return a.first.time_since_epoch().count() < b.first.time_since_epoch().count();});

        size_t from_itor = searchPoint(from, true);
        size_t to_itor = searchPoint(to, false);

        qjson::JObject returnJson(qjson::JValueType::JList);
        for (auto i = from_itor; i <= to_itor; i++)
        {
            switch (m_message_queue[i].second.type)
            {
            case MessageStruct::MessageType::NOMAL_MESSAGE:
            {
                const auto& messageStruct = m_message_queue[i].second;
                qjson::JObject localjson;
                localjson["type"] = "group_message";
                localjson["data"]["user_id"] = messageStruct.user_id;
                localjson["data"]["group_id"] = this->m_group_id;
                localjson["data"]["message"] = messageStruct.message;
                localjson["time_point"] = m_message_queue[i].first.time_since_epoch().count();
                returnJson.push_back(std::move(localjson));
            }
                break;
            case MessageStruct::MessageType::TIP_MESSAGE:
            {
                const auto& messageStruct = m_message_queue[i].second;
                qjson::JObject localjson;
                localjson["type"] = "group_tip_message";
                localjson["data"]["user_id"] = messageStruct.user_id;
                localjson["data"]["group_id"] = this->m_group_id;
                localjson["data"]["message"] = messageStruct.message;
                localjson["time_point"] = m_message_queue[i].first.time_since_epoch().count();
                returnJson.push_back(std::move(localjson));
            }
                break;
            default:
                break;
            }
        }

        sendData(qjson::JWriter::fastWrite(returnJson));
        co_return;
    }

    bool GroupRoom::hasUser(long long user_id) const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
        return m_user_id_map.find(user_id) != m_user_id_map.end();
    }

    std::unordered_map<long long, GroupRoom::UserDataStruct> GroupRoom::getUserList() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
        return m_user_id_map;
    }

    std::string GroupRoom::getUserNickname(long long user_id) const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
        auto itor = m_user_id_map.find(user_id);
        if (itor == m_user_id_map.end())
            throw std::logic_error("The user isn't in the room");

        return itor->second.nickname;
    }

    long long GroupRoom::getUserGroupLevel(long long user_id) const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
        auto itor = m_user_id_map.find(user_id);
        if (itor == m_user_id_map.end())
            throw std::logic_error("The user isn't in the room");

        return itor->second.groupLevel;
    }

    std::unordered_map<long long,
        GroupPermission::PermissionType> GroupRoom::getUserPermissionList() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        return std::move(this->m_permission.getUserPermissionList());
    }

    long long GroupRoom::getAdministrator() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        std::shared_lock<std::shared_mutex> sl(m_administrator_user_id_mutex);
        return m_administrator_user_id;
    }

    void GroupRoom::setAdministrator(long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");

        std::unique_lock<std::shared_mutex> ul1(m_user_id_map_mutex, std::defer_lock);
        std::unique_lock<std::shared_mutex> ul2(m_administrator_user_id_mutex, std::defer_lock);
        std::lock(ul1, ul2);

        if (m_administrator_user_id == 0)
        {
            auto itor = m_user_id_map.find(user_id);
            if (itor == m_user_id_map.end())
            {
                m_user_id_map[user_id] = UserDataStruct{
                    serverManager.getUser(user_id)->getUserName(), 1 };
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

    std::vector<long long> GroupRoom::getDefaultUserList() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        return std::move(this->m_permission.getDefaultUserList());
    }

    std::vector<long long> GroupRoom::getOperatorList() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        return std::move(this->m_permission.getOperatorList());
    }

    bool GroupRoom::muteUser(long long executorId, long long user_id, const std::chrono::minutes& mins)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (executorId == user_id ||
            !this->hasUser(user_id) ||
            !this->hasUser(executorId))
            return false;

        auto executorIdType = this->m_permission.getUserPermissionType(executorId);
        auto userIdType = this->m_permission.getUserPermissionType(user_id);
        if (userIdType >= executorIdType)
            return false;

        std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
        m_muted_user_map[user_id] = std::pair<std::chrono::time_point<std::chrono::system_clock,
            std::chrono::milliseconds>,
            std::chrono::minutes>{ std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now()),
            mins };
        ul.unlock();

        asio::io_context io_context;
        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        asio::co_spawn(io_context, this->sendTipMessage(executorId, std::format("{} was muted by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname)),
            asio::use_awaitable);
        sl.unlock();
        io_context.run();

        return true;
    }

    bool GroupRoom::unmuteUser(long long executorId, long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (executorId == user_id ||
            !this->hasUser(user_id) ||
            !this->hasUser(executorId))
            return false;

        auto executorIdType = this->m_permission.getUserPermissionType(executorId);
        auto userIdType = this->m_permission.getUserPermissionType(user_id);
        if (userIdType >= executorIdType)
            return false;

        std::unique_lock<std::shared_mutex> ul(m_muted_user_map_mutex);
        m_muted_user_map.erase(user_id);
        ul.unlock();

        asio::io_context io_context;
        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        asio::co_spawn(io_context, this->sendTipMessage(executorId, std::format("{} was unmuted by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname)),
            asio::use_awaitable);
        sl.unlock();
        io_context.run();

        return true;
    }

    bool GroupRoom::kickUser(long long executorId, long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (executorId == user_id ||
            !this->hasUser(user_id) ||
            !this->hasUser(executorId))
            return false;

        auto executorIdType = this->m_permission.getUserPermissionType(executorId);
        auto userIdType = this->m_permission.getUserPermissionType(user_id);
        if (userIdType >= executorIdType)
            return false;

        std::unique_lock<std::shared_mutex> sl1(m_user_id_map_mutex, std::defer_lock),
            sl2(m_muted_user_map_mutex, std::defer_lock);

        asio::io_context io_context;
        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        asio::co_spawn(io_context, this->sendTipMessage(executorId, std::format("{} was kicked by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname)),
            asio::use_awaitable);
        sl.unlock();
        io_context.run();

        return true;
    }

    bool GroupRoom::addOperator(long long executorId, long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (executorId == user_id ||
            !this->hasUser(user_id) ||
            !this->hasUser(executorId))
            return false;

        if (this->m_permission.getUserPermissionType(executorId) !=
            GroupPermission::PermissionType::Administrator)
            return false;
        if (this->m_permission.getUserPermissionType(user_id) !=
            GroupPermission::PermissionType::Default)
            return false;

        this->m_permission.modifyUserPermission(user_id,
            GroupPermission::PermissionType::Operator);

        asio::io_context io_context;
        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        asio::co_spawn(io_context, this->sendTipMessage(executorId, std::format("{} was turned operator by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname)),
            asio::use_awaitable);
        sl.unlock();
        io_context.run();

        return true;
    }

    bool GroupRoom::removeOperator(long long executorId, long long user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (executorId == user_id ||
            !this->hasUser(user_id) ||
            !this->hasUser(executorId))
            return false;

        if (this->m_permission.getUserPermissionType(executorId) !=
            GroupPermission::PermissionType::Administrator)
            return false;
        if (this->m_permission.getUserPermissionType(user_id) !=
            GroupPermission::PermissionType::Operator)
            return false;

        this->m_permission.modifyUserPermission(user_id,
            GroupPermission::PermissionType::Default);

        asio::io_context io_context;
        std::shared_lock<std::shared_mutex> sl(this->m_user_id_map_mutex);
        asio::co_spawn(io_context, this->sendTipMessage(executorId, std::format("{} was turned default user by {}",
            this->m_user_id_map[user_id].nickname, this->m_user_id_map[executorId].nickname)),
            asio::use_awaitable);
        sl.unlock();
        io_context.run();

        return true;
    }

    void GroupRoom::removeThisRoom()
    {
        m_can_be_used = false;

        {
            // sql 上面删除此房间
        }

        // 剩下其他东西
    }

    bool GroupRoom::canBeUsed() const
    {
        return m_can_be_used;
    }
}