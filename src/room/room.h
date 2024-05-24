#ifndef ROOM_H
#define ROOM_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <functional>

#include "socket.h"

namespace qls
{
    enum class Equipment
    {
        Unknown,
        Windows,
        Unix,
        Android,
        Ios
    };

    struct BaseUserSetting
    {
        long long user_id;
        Equipment equipment = Equipment::Unknown;
    };

    /*
    * @brief 基类房间
    */
    class BaseRoom
    {
    public:
        BaseRoom() = default;
        virtual ~BaseRoom() = default;

        virtual bool joinRoom(
            const std::shared_ptr<Socket>& socket_ptr,
            const BaseUserSetting& user);

        virtual bool leaveRoom(
            const std::shared_ptr<Socket>& socket_ptr);

        virtual asio::awaitable<bool> sendData(const std::string& data);
        virtual asio::awaitable<bool> sendData(const std::string& data, long long user_id);

    private:
        std::unordered_map<std::shared_ptr<Socket>,
            BaseUserSetting>    m_userMap;
        std::shared_mutex       m_userMap_mutex;

        std::queue<std::shared_ptr<Socket>>
                                m_userDeleteQueue;
        std::shared_mutex       m_userDeleteQueue_mutex;
    };
}

#endif // !ROOM_H
