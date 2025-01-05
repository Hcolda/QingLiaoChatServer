#include "privateRoom.h"

#include <stdexcept>
#include <vector>
#include <atomic>
#include <shared_mutex>

#include <Json.h>
#include "qls_error.h"

static std::pmr::synchronized_pool_resource local_sync_private_room_pool;

namespace qls
{

struct PrivateRoomImpl
{
    const UserID m_user_id_1, m_user_id_2;

    std::atomic<bool>               m_can_be_used;

    std::vector<std::pair<std::chrono::time_point<std::chrono::system_clock,
        std::chrono::milliseconds>,
        MessageStructure>>          m_message_queue;
    mutable std::shared_mutex       m_message_queue_mutex;
};

void PrivateRoomImplDeleter::operator()(PrivateRoomImpl* pri) noexcept
{
    local_sync_private_room_pool.deallocate(pri, sizeof(PrivateRoomImpl));
}

// PrivateRoom
PrivateRoom::PrivateRoom(UserID user_id_1, UserID user_id_2, bool is_create):
    TextDataRoom(&local_sync_private_room_pool),
    m_impl(
        [&](PrivateRoomImpl* pri) {
            new(pri) PrivateRoomImpl(user_id_1, user_id_2); return pri;
            } (static_cast<PrivateRoomImpl*>(
                local_sync_private_room_pool.allocate(sizeof(PrivateRoomImpl)))))
{
    if (is_create) {
        // sql 创建private room
        m_impl->m_can_be_used = true;
    } else {
        // sql 读取private room
        m_impl->m_can_be_used = true;
    }

    TextDataRoom::joinRoom(user_id_1);
    TextDataRoom::joinRoom(user_id_2);
}

void PrivateRoom::sendMessage(std::string_view message, UserID sender_user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::private_room_unable_to_use));
    if (!hasUser(sender_user_id))
        return;

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
    json["type"] = "private_message";
    json["data"]["user_id"] = sender_user_id.getOriginValue();
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json));
}

void PrivateRoom::sendTipMessage(std::string_view message, UserID sender_user_id)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::private_room_unable_to_use));
    if (!hasUser(sender_user_id))
        return;
    
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
    json["type"] = "private_tip_message";
    json["data"]["user_id"] = sender_user_id.getOriginValue();
    json["data"]["message"] = message;

    sendData(qjson::JWriter::fastWrite(json));
}

void PrivateRoom::getMessage(const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to)
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::private_room_unable_to_use));
    if (from > to) return;

    auto searchPoint = [this](
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& p,
        bool edge = false) -> std::size_t {

            std::size_t left = 0ull;
            std::size_t right = m_impl->m_message_queue.size() - 1;
            std::size_t middle = (left + right) / 2;

            while (left < right - 1) {
                if (m_impl->m_message_queue[middle].first.time_since_epoch().count() ==
                    p.time_since_epoch().count())
                    return middle;
                else if (m_impl->m_message_queue[middle].first.time_since_epoch().count() <
                    p.time_since_epoch().count()) {
                    left = middle;
                    middle = (left + right) / 2;
                }
                else {
                    right = middle;
                    middle = (left + right) / 2;
                }
            }

            return edge ? left : right;
        };

    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_message_queue_mutex);
    if (m_impl->m_message_queue.empty())
    {
        sendData(
            qjson::JWriter::fastWrite(qjson::JObject(qjson::JValueType::JList)));
        return;
    }

    std::sort(m_impl->m_message_queue.begin(), m_impl->m_message_queue.end(), [](
        const std::pair<std::chrono::system_clock::time_point, MessageStructure>& a,
        const std::pair<std::chrono::system_clock::time_point, MessageStructure>& b)
        {return a.first.time_since_epoch().count() < b.first.time_since_epoch().count(); });

    std::size_t from_itor = searchPoint(from, true);
    std::size_t to_itor = searchPoint(to, false);

    qjson::JObject returnJson(qjson::JValueType::JList);
    for (auto i = from_itor; i <= to_itor; i++) {
        switch (m_impl->m_message_queue[i].second.type) {
        case MessageType::NOMAL_MESSAGE: {
            const auto& MessageStructure = m_impl->m_message_queue[i].second;
            qjson::JObject localjson;
            localjson["type"] = "private_message";
            localjson["data"]["user_id"] = MessageStructure.user_id.getOriginValue();
            localjson["data"]["message"] = MessageStructure.message;
            localjson["time_point"] = m_impl->m_message_queue[i].first.time_since_epoch().count();
            returnJson.push_back(std::move(localjson));
            break;
        }
        case MessageType::TIP_MESSAGE: {
            const auto& MessageStructure = m_impl->m_message_queue[i].second;
            qjson::JObject localjson;
            localjson["type"] = "private_tip_message";
            localjson["data"]["user_id"] = MessageStructure.user_id.getOriginValue();
            localjson["data"]["message"] = MessageStructure.message;
            localjson["time_point"] = m_impl->m_message_queue[i].first.time_since_epoch().count();
            returnJson.push_back(std::move(localjson));
            break;
        }
        default:
            break;
        }
    }

    sendData(qjson::JWriter::fastWrite(returnJson));
}

std::pair<UserID, UserID> PrivateRoom::getUserID() const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::private_room_unable_to_use));
    return {m_impl->m_user_id_1, m_impl->m_user_id_2};
}

bool PrivateRoom::hasMember(UserID user_id) const
{
    if (!m_impl->m_can_be_used)
        throw std::system_error(make_error_code(qls_errc::private_room_unable_to_use));
    return user_id == m_impl->m_user_id_1 || user_id == m_impl->m_user_id_2;
}

void PrivateRoom::removeThisRoom()
{
    m_impl->m_can_be_used = false;

    {
        // sql 上面删除此房间
    }

    // 剩下其他东西
}

bool PrivateRoom::canBeUsed() const
{
    return m_impl->m_can_be_used;
}

} // namespace qls
