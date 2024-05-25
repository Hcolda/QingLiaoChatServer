#include "privateRoom.h"

#include <stdexcept>

#include <Json.h>

namespace qls
{
    // PrivateRoom
    PrivateRoom::PrivateRoom(long long user_id_1, long long user_id_2, bool is_create) :
        m_user_id_1(user_id_1),
        m_user_id_2(user_id_2)
    {
        if (is_create)
        {
            // sql 创建private room
            m_can_be_used = true;
        }
        else
        {
            // sql 读取private room
            m_can_be_used = true;
        }
    }

    asio::awaitable<void> PrivateRoom::sendMessage(const std::string& message,
        long long sender_user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (!hasUser(sender_user_id))
            co_return;

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
        json["type"] = "private_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["message"] = message;

        co_return co_await sendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<void> PrivateRoom::sendTipMessage(const std::string& message,
        long long sender_user_id)
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        if (!hasUser(sender_user_id))
            co_return;
        
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
        json["type"] = "private_tip_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["message"] = message;

        co_return co_await sendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<void> PrivateRoom::getMessage(const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from, const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to)
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
            co_return co_await sendData(
                qjson::JWriter::fastWrite(qjson::JObject(qjson::JValueType::JList)));
        }

        std::sort(m_message_queue.begin(), m_message_queue.end(), [](
            const std::pair<std::chrono::system_clock::time_point, MessageStruct>& a,
            const std::pair<std::chrono::system_clock::time_point, MessageStruct>& b)
            {return a.first.time_since_epoch().count() < b.first.time_since_epoch().count(); });

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
                localjson["type"] = "private_message";
                localjson["data"]["user_id"] = messageStruct.user_id;
                localjson["data"]["message"] = messageStruct.message;
                localjson["time_point"] = m_message_queue[i].first.time_since_epoch().count();
                returnJson.push_back(std::move(localjson));
            }
            break;
            case MessageStruct::MessageType::TIP_MESSAGE:
            {
                const auto& messageStruct = m_message_queue[i].second;
                qjson::JObject localjson;
                localjson["type"] = "private_tip_message";
                localjson["data"]["user_id"] = messageStruct.user_id;
                localjson["data"]["message"] = messageStruct.message;
                localjson["time_point"] = m_message_queue[i].first.time_since_epoch().count();
                returnJson.push_back(std::move(localjson));
            }
            break;
            default:
                break;
            }
        }

        co_return co_await sendData(qjson::JWriter::fastWrite(returnJson));
    }

    long long PrivateRoom::getUserID1() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        return m_user_id_1;
    }

    long long PrivateRoom::getUserID2() const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        return m_user_id_1;
    }

    bool PrivateRoom::hasUser(long long user_id) const
    {
        if (!this->m_can_be_used) throw std::logic_error("This room can't be used");
        return user_id == m_user_id_1 || user_id == m_user_id_2;
    }

    void PrivateRoom::removeThisRoom()
    {
        m_can_be_used = false;

        {
            // sql 上面删除此房间
        }

        // 剩下其他东西
    }

    bool PrivateRoom::canBeUsed() const
    {
        return m_can_be_used;
    }
}