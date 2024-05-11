#include "room.h"

#include <stdexcept>

#include <QuqiCrypto.hpp>
#include <Json.h>

namespace qls
{
    bool BaseRoom::baseJoinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user)
    {
        if (!socket_ptr) return false;
        else if (!user.sendFunction) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        m_userMap[socket_ptr] = user;
        return true;
    }

    bool BaseRoom::baseLeaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr)
    {
        if (!socket_ptr) return false;

        std::lock_guard<std::shared_mutex> lock(m_userMap_mutex);
        if (m_userMap.find(socket_ptr) == m_userMap.end()) return false;
            m_userMap.erase(m_userMap.find(socket_ptr));

        return true;
    }

    asio::awaitable<void> BaseRoom::baseSendData(const std::string& data)
    {
        // 广播数据
        {
            qcrypto::AES<qcrypto::AESMode::CBC_256> aes;
            std::shared_lock<std::shared_mutex> sharedLock(m_userMap_mutex);

            for (auto i = m_userMap.begin(); i != m_userMap.end(); i++)
            {
                try
                {
                    /*std::string out;
                    if (!aes.encrypt(data, out, { i->second.key, 32 }, { i->second.iv, 16 }, true))
                        throw std::logic_error("Key and ivec of AES are invalid");
                    co_await i->first->async_send(asio::buffer(out), asio::use_awaitable);*/

                    co_await i->second.sendFunction(data, 0, 1, -1);
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
                    std::shared_ptr<asio::ip::tcp::socket> localSocket_ptr = std::move(m_userDeleteQueue.front());
                    auto itor = m_userMap.find(localSocket_ptr);
                    if (itor != m_userMap.end())
                    {
                        m_userMap.erase(itor);
                    }

                    m_userDeleteQueue.pop();
                } while (!m_userDeleteQueue.empty());
            }
        }

        co_return;
    }

    void BaseRoom::baseSendData(const std::string& data, std::function<void(std::error_code, size_t)>)
    {
        
    }

    asio::awaitable<void> BaseRoom::baseSendData(const std::string& data, long long user_id)
    {
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
                        /*std::string out;
                        if (!aes.encrypt(data, out, { i->second.key, 32 }, { i->second.iv, 16 }, true))
                            throw std::logic_error("Key and ivec of AES are invalid");
                        co_await i->first->async_send(asio::buffer(out), asio::use_awaitable);*/

                        co_await i->second.sendFunction(data, 0, 1, -1);
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
                    std::shared_ptr<asio::ip::tcp::socket> localSocket_ptr = std::move(m_userDeleteQueue.front());
                    auto itor = m_userMap.find(localSocket_ptr);
                    if (itor != m_userMap.end())
                    {
                        m_userMap.erase(itor);
                    }

                    m_userDeleteQueue.pop();
                } while (!m_userDeleteQueue.empty());
            }
        }

        co_return;
    }

    void BaseRoom::baseSendData(const std::string& data, long long user_id, std::function<void(std::error_code, size_t)>)
    {
        
    }
}
