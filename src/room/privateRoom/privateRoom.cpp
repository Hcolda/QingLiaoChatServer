#include "privateRoom.h"

#include <stdexcept>

#include <QuqiCrypto.hpp>
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
        }
        else
        {
            // sql 读取private room
        }
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
}