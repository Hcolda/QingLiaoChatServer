#include "room.h"

#include <mutex>
#include <shared_mutex>
#include <queue>
#include <unordered_map>
#include <list>

#include <Json.h>

#include "dataPackage.h"

namespace qls
{
    struct SingleUserStruct
    {
        std::list<std::shared_ptr<Socket>> sockets;
        std::shared_mutex mutex;
    };

    struct BaseRoomImpl
    {
        std::unordered_map<long long,
            SingleUserStruct>
                                m_userMap;
        std::shared_mutex       m_userMap_mutex;

        std::queue<std::pair<long long, std::shared_ptr<Socket>>>
                                m_userDeleteQueue;
        std::shared_mutex       m_userDeleteQueue_mutex;
    };

    BaseRoom::BaseRoom():
        m_impl(std::make_shared<BaseRoomImpl>())
    {
    }

    bool BaseRoom::joinRoom(
        const std::shared_ptr<Socket>& socket_ptr,
        long long user_id)
    {
        if (!socket_ptr) return false;

        {
            std::lock_guard<std::shared_mutex> lock(m_impl->m_userMap_mutex);
            if (m_impl->m_userMap.find(user_id) == m_impl->m_userMap.cend())
                m_impl->m_userMap.try_emplace(user_id);
        }
        {
            std::shared_lock<std::shared_mutex> userLock(m_impl->m_userMap_mutex);
            auto iter = m_impl->m_userMap.find(user_id);
            if (iter == m_impl->m_userMap.cend())
                return false;
            {
                std::lock_guard<std::shared_mutex> singleLock(iter->second.mutex);
                iter->second.sockets.push_front(socket_ptr);
            }
        }
        
        return true;
    }

    bool BaseRoom::leaveRoom(long long user_id,
        const std::shared_ptr<Socket>& socket_ptr)
    {
        if (!socket_ptr) return false;

        {
            std::shared_lock<std::shared_mutex> userLock(m_impl->m_userMap_mutex);
            auto iter = m_impl->m_userMap.find(user_id);
            if (iter == m_impl->m_userMap.cend())
                return false;
            {
                std::unique_lock<std::shared_mutex> singleLock(iter->second.mutex);
                auto list_iter = std::find(iter->second.sockets.begin(), iter->second.sockets.end(), socket_ptr);
                if (list_iter == iter->second.sockets.cend()) return false;
                iter->second.sockets.erase(list_iter);
            }
        }

        return true;
    }

    asio::awaitable<void> BaseRoom::sendData(const std::string& data)
    {
        auto executor = co_await asio::this_coro::executor;
        bool has_closed_socket = false;

        // 广播数据
        {
            std::shared_lock<std::shared_mutex> sharedLock(m_impl->m_userMap_mutex);

            for (auto i = m_impl->m_userMap.begin(); i != m_impl->m_userMap.end(); ++i)
            {
                std::shared_lock<std::shared_mutex> singleLock(i->second.mutex);
                for (auto j = i->second.sockets.begin(); j != i->second.sockets.end(); ++j)
                {
                    try
                    {
                        auto pack = qls::DataPackage::makePackage(data);
                        pack->requestID = 0;
                        pack->type = 1;
                        pack->sequence = -1;
                        co_await asio::async_write(*(*j), asio::buffer(pack->packageToString()), asio::use_awaitable);
                    }
                    catch (const std::system_error&)
                    {
                        has_closed_socket = true;
                        std::lock_guard<std::shared_mutex> lock(m_impl->m_userDeleteQueue_mutex);
                        m_impl->m_userDeleteQueue.push({ i->first, *j });
                    }
                    catch(...) {}
                }
            }
        }

        // 去除已经关闭的连接
        if (has_closed_socket)
        {
            asio::co_spawn(executor, [this]()->asio::awaitable<void> {
                std::unique_lock<std::shared_mutex>
                    lock(m_impl->m_userDeleteQueue_mutex, std::defer_lock);
                std::shared_lock<std::shared_mutex>
                    lock2(m_impl->m_userMap_mutex, std::defer_lock);
                std::lock(lock, lock2);

                if (!m_impl->m_userDeleteQueue.empty())
                {
                    do
                    {
                        auto [local_user_id, localSocket_ptr] =
                            std::move(m_impl->m_userDeleteQueue.front());

                        auto iter = m_impl->m_userMap.find(local_user_id);
                        if (iter != m_impl->m_userMap.cend())
                        {
                            std::unique_lock<std::shared_mutex> singleLock(iter->second.mutex);
                            auto list_iter = std::find(iter->second.sockets.begin(), iter->second.sockets.end(), localSocket_ptr);
                            if (list_iter != iter->second.sockets.cend())
                                iter->second.sockets.erase(list_iter);
                        }

                        m_impl->m_userDeleteQueue.pop();
                    } while (!m_impl->m_userDeleteQueue.empty());
                }
                co_return;
                }, asio::detached);
        }

        co_return;
    }

    void BaseRoom::sendData(const std::string& data, std::function<void(std::error_code, size_t)>)
    {
        
    }

    asio::awaitable<void> BaseRoom::sendData(const std::string& data, long long user_id)
    {
        auto executor = co_await asio::this_coro::executor;
        bool has_closed_socket = false;

        // 广播数据
        {
            std::shared_lock<std::shared_mutex> sharedLock(m_impl->m_userMap_mutex);

            auto i = m_impl->m_userMap.find(user_id);
            std::shared_lock<std::shared_mutex> singleLock(i->second.mutex);
            for (auto j = i->second.sockets.begin(); j != i->second.sockets.end(); ++j)
            {
                try
                {
                    auto pack = qls::DataPackage::makePackage(data);
                    pack->requestID = 0;
                    pack->type = 1;
                    pack->sequence = -1;
                    co_await asio::async_write(*(*j), asio::buffer(pack->packageToString()), asio::use_awaitable);
                }
                catch (const std::system_error&)
                {
                    has_closed_socket = true;
                    std::lock_guard<std::shared_mutex> lock(m_impl->m_userDeleteQueue_mutex);
                    m_impl->m_userDeleteQueue.push({ i->first, *j });
                }
                catch (...) {}
            }
        }

        // 去除已经关闭的连接
        if (has_closed_socket)
        {
            asio::co_spawn(executor, [this]()->asio::awaitable<void> {
                std::unique_lock<std::shared_mutex>
                    lock(m_impl->m_userDeleteQueue_mutex, std::defer_lock);
                std::shared_lock<std::shared_mutex>
                    lock2(m_impl->m_userMap_mutex, std::defer_lock);
                std::lock(lock, lock2);

                if (!m_impl->m_userDeleteQueue.empty())
                {
                    do
                    {
                        auto [local_user_id, localSocket_ptr] =
                            std::move(m_impl->m_userDeleteQueue.front());

                        auto iter = m_impl->m_userMap.find(local_user_id);
                        if (iter != m_impl->m_userMap.cend())
                        {
                            std::unique_lock<std::shared_mutex> singleLock(iter->second.mutex);
                            auto list_iter = std::find(iter->second.sockets.begin(), iter->second.sockets.end(), localSocket_ptr);
                            if (list_iter != iter->second.sockets.cend())
                                iter->second.sockets.erase(list_iter);
                        }

                        m_impl->m_userDeleteQueue.pop();
                    } while (!m_impl->m_userDeleteQueue.empty());
                }
                co_return;
                }, asio::detached);
        }

        co_return;
    }

    void BaseRoom::sendData(const std::string& data, long long user_id, std::function<void(std::error_code, size_t)>)
    {
        // 广播数据给单个user
        std::shared_lock<std::shared_mutex> sharedLock(m_impl->m_userMap_mutex);
        auto i = m_impl->m_userMap.find(user_id);
        {
            std::shared_lock<std::shared_mutex> singleLock(i->second.mutex);

            for (auto j = i->second.sockets.begin(); j != i->second.sockets.end();)
            {
                auto pack = qls::DataPackage::makePackage(data);
                pack->requestID = 0;
                pack->type = 1;
                pack->sequence = -1;
                asio::async_write(*(*j), asio::buffer(pack->packageToString()),
                    [this, user_id, socket_ptr = (*j)](std::error_code e, size_t n) -> void {
                        if (e)
                        {
                            std::shared_lock<std::shared_mutex> userLock(m_impl->m_userMap_mutex);
                            auto iter = m_impl->m_userMap.find(user_id);
                            {
                                std::unique_lock<std::shared_mutex> singleLock(iter->second.mutex);
                                auto list_iter = std::find(iter->second.sockets.begin(), iter->second.sockets.end(), socket_ptr);
                                if (list_iter != iter->second.sockets.cend())
                                    iter->second.sockets.erase(list_iter);
                            }
                        }
                    });
            }
        }
    }
}
