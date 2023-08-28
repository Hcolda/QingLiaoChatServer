#include "room.h"

namespace qls
{
    bool BaseRoom::joinBaseRoom(std::shared_ptr<asio::ip::tcp::socket> socket_ptr, const User& user)
    {
        if (socket_ptr.get() == nullptr) return false;
        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        m_userMap[socket_ptr] = user;
        return true;
    }

    bool BaseRoom::leaveBaseRoom(std::shared_ptr<asio::ip::tcp::socket> socket_ptr)
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
                    co_await i->first->async_send(asio::buffer(data), asio::use_awaitable);
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
