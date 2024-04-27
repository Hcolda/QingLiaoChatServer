#include "room.h"

#include <stdexcept>

#include <QuqiCrypto.hpp>
#include <Json.h>

namespace qls
{
    bool BaseRoom::joinRoom(
        const std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>& socket_ptr,
        const BaseUserSetting& user)
    {
        if (!socket_ptr) return false;
        else if (!user.sendFunction) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        m_userMap[socket_ptr] = user;
        return true;
    }

    bool BaseRoom::leaveRoom(
        const std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>& socket_ptr)
    {
        if (!socket_ptr) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        if (m_userMap.find(socket_ptr) == m_userMap.end()) return false;
            m_userMap.erase(m_userMap.find(socket_ptr));

        return true;
    }

    asio::awaitable<bool> BaseRoom::sendData(const std::string& data)
    {
        bool result = false;

        // 广播数据
        {
            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                try
                {
                    result = (co_await i->second.sendFunction(data, 0, 1, -1) == data.size())
                        ? true : false;
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
                    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>
                        localSocket_ptr = std::move(m_userDeleteQueue.front());

                    auto itor = m_userMap.find(localSocket_ptr);
                    if (itor != m_userMap.end())
                    {
                        m_userMap.erase(itor);
                    }

                    m_userDeleteQueue.pop();
                } while (!m_userDeleteQueue.empty());
            }
        }

        co_return result;
    }

    asio::awaitable<bool> BaseRoom::sendData(const std::string& data, long long user_id)
    {
        bool result = false;

        // 广播数据给单个user
        {
            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                if (i->second.user_id == user_id)
                {
                    try
                    {
                        result = (co_await i->second.sendFunction(data, 0, 1, -1) == data.size())
                            ? true : false;
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
                    std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>
                        localSocket_ptr = std::move(m_userDeleteQueue.front());

                    auto itor = m_userMap.find(localSocket_ptr);
                    if (itor != m_userMap.end())
                    {
                        m_userMap.erase(itor);
                    }

                    m_userDeleteQueue.pop();
                } while (!m_userDeleteQueue.empty());
            }
        }

        co_return result;
    }
}
