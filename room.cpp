#include "room.h"

#include <QuqiCrypto.hpp>
#include <Json.h>

namespace qls
{
    bool BaseRoom::joinBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user)
    {
        if (socket_ptr.get() == nullptr) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        m_userMap[socket_ptr] = user;
        return true;
    }

    bool BaseRoom::leaveBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        if (socket_ptr.get() == nullptr) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        if (m_userMap.find(socket_ptr) == m_userMap.end()) return false;
            m_userMap.erase(m_userMap.find(socket_ptr));

        return true;
    }

    asio::awaitable<bool> BaseRoom::baseSendData(const std::string& data)
    {
        bool queueEmpty = true;
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
                    m_userDeleteQueue.push(i->first);
                }
            }
            queueEmpty = m_userDeleteQueue.empty();
        }

        // 去除已经关闭的连接
        {
            if (queueEmpty)
            {
                std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
                while (!m_userDeleteQueue.empty())
                {
                    std::shared_ptr<asio::ip::tcp::socket> localSocket_ptr = std::move(m_userDeleteQueue.front());
                    if (m_userMap.find(localSocket_ptr) != m_userMap.end())
                    {
                        m_userMap.erase(m_userMap.find(localSocket_ptr));
                    }

                    m_userDeleteQueue.pop();
                }
            }
        }

        co_return true;
    }

    asio::awaitable<bool> BaseRoom::baseSendData(const std::string& data, long long user_id)
    {
        bool queueEmpty = true;
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
                        m_userDeleteQueue.push(i->first);
                    }
                }
            }
            queueEmpty = m_userDeleteQueue.empty();
        }

        // 去除已经关闭的连接
        {
            if (queueEmpty)
            {
                std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
                while (!m_userDeleteQueue.empty())
                {
                    std::shared_ptr<asio::ip::tcp::socket> localSocket_ptr = std::move(m_userDeleteQueue.front());
                    if (m_userMap.find(localSocket_ptr) != m_userMap.end())
                    {
                        m_userMap.erase(m_userMap.find(localSocket_ptr));
                    }

                    m_userDeleteQueue.pop();
                }
            }
        }

        co_return true;
    }

    // BasePrivateRoom

    BasePrivateRoom::BasePrivateRoom(long long user_id_1, long long user_id_2) :
        m_user_id_1(user_id_1),
        m_user_id_2(user_id_2)
    {}

    bool BasePrivateRoom::joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user)
    {
        if (user.user_id != m_user_id_1 && user.user_id != m_user_id_2)
            return false;
        return joinBaseRoom(socket_ptr, user);
    }

    bool BasePrivateRoom::leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        return leaveBaseRoom(socket_ptr);
    }

    asio::awaitable<bool> BasePrivateRoom::sendMessage(const std::string& message, long long sender_user_id)
    {
        if (sender_user_id != m_user_id_1 && sender_user_id != m_user_id_2)
            co_return false;

        qjson::JObject json;
        json["type"] = "private_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> BasePrivateRoom::sendTipMessage(const std::string& message)
    {
        qjson::JObject json;
        json["type"] = "private_tip_message";
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    // BaseGroupRoom

    BaseGroupRoom::BaseGroupRoom(long long group_id) :
        m_group_id(group_id) {}

    bool BaseGroupRoom::baseAddMember(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) == m_user_id_map.end())
            m_user_id_map.insert(user_id);

        return true;
    }

    bool BaseGroupRoom::baseRemoveMember(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_id_map_mutex);
        if (m_user_id_map.find(user_id) != m_user_id_map.end())
            m_user_id_map.erase(user_id);

        return true;
    }

    bool BaseGroupRoom::joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user)
    {
        return joinBaseRoom(socket_ptr, user);
    }

    bool BaseGroupRoom::leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        return leaveBaseRoom(socket_ptr);
    }

    asio::awaitable<bool> BaseGroupRoom::sendMessage(const std::string& message, long long sender_user_id)
    {
        // 是否有此user_id
        {
            std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
            if (m_user_id_map.find(sender_user_id) == m_user_id_map.end())
                co_return false;
        }

        qjson::JObject json;
        json["type"] = "group_message";
        json["data"]["user_id"] = sender_user_id;
        json["data"]["group_id"] = m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> BaseGroupRoom::sendTipMessage(const std::string& message)
    {
        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["group_id"] = m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }

    asio::awaitable<bool> BaseGroupRoom::sendUserTipMessage(const std::string& message, long long receiver_user_id)
    {
        {
            std::shared_lock<std::shared_mutex> sl(m_user_id_map_mutex);
            if (m_user_id_map.find(receiver_user_id) == m_user_id_map.end())
                co_return false;
        }

        qjson::JObject json;
        json["type"] = "group_tip_message";
        json["data"]["group_id"] = m_group_id;
        json["data"]["message"] = message;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json), receiver_user_id);
    }
}
