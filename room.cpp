#include "room.h"

#include <QuqiCrypto.hpp>

namespace room
{
    bool BaseRoom::joinBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUser& user)
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
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);
            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                try
                {
                    qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
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
}

#include <Json.h>

namespace qls
{
    PrivateRoom::PrivateRoom(long long user_id_1, long long user_id_2) :
        m_user_id_1(user_id_1),
        m_user_id_2(user_id_2)
    {
    }

    bool PrivateRoom::joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user)
    {
        if (user.id != m_user_id_1 || user.id != m_user_id_2)
            return false;
        return joinBaseRoom(socket_ptr, user);
    }

    bool PrivateRoom::leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        return leaveBaseRoom(socket_ptr);
    }

    asio::awaitable<bool> PrivateRoom::sendData(const std::string& message, long long user_id)
    {
        qjson::JObject json;
        json["type"] = "private_message";
        json["data"]["user_id"] = user_id;
        json["data"]["message"] = message;
        std::string out;

        co_return co_await baseSendData(qjson::JWriter::fastWrite(json));
    }
}
