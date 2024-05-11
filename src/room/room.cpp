#include "room.h"

#include <stdexcept>

#include <Json.h>

#include "dataPackage.h"

namespace qls
{
    bool BaseRoom::joinRoom(
        const std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>& socket_ptr,
        const BaseUserSetting& user)
    {
        if (!socket_ptr) return false;

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
        bool result = true;

        // 广播数据
        {
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                try
                {
                    auto pack = qls::DataPackage::makePackage(data);
                    pack->requestID = 0;
                    pack->type = 1;
                    pack->sequence = -1;
                    asio::async_write(*(i->first), asio::buffer(pack->packageToString()), asio::detached);
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
        bool result = true;

        // 广播数据给单个user
        {
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                if (i->second.user_id == user_id)
                {
                    try
                    {
                        auto pack = qls::DataPackage::makePackage(data);
                        pack->requestID = 0;
                        pack->type = 1;
                        pack->sequence = -1;
                        asio::async_write(*(i->first), asio::buffer(pack->packageToString()), asio::detached);
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
