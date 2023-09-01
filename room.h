#pragma once

#include <asio.hpp>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <queue>
#include "network.h"

namespace qls
{
    class BaseRoom
    {
    public:
        enum class Equipment
        {
            Unknown,
            Windows,
            Unix,
            Android,
            Ios
        };

        struct User
        {
            long long id;
            Equipment equipment = Equipment::Unknown;
        };

        BaseRoom() = default;
        ~BaseRoom() = default;

        /*
        * @brief 加入房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true成功 false失败
        */
        bool joinBaseRoom(std::shared_ptr<asio::ip::tcp::socket> socket_ptr, const User& user);
        
        /*
        * @brief 离开房间
        * @param socket_ptr socket指针
        * @return true成功 false失败
        */
        bool leaveBaseRoom(std::shared_ptr<asio::ip::tcp::socket> socket_ptr);
        
        /*
        * @brief 发送消息给类中所有用户（广播）
        * @param data 二进制数据
        * @return 协程 true成功 false失败
        */
        asio::awaitable<bool> baseSendData(const std::string& data);

    private:
        std::unordered_map<std::shared_ptr<asio::ip::tcp::socket>, User>    m_userMap;
        std::queue<std::shared_ptr<asio::ip::tcp::socket>>                  m_userDeleteQueue;
        std::shared_mutex                                                   m_userMap_mutex;
    };
}


