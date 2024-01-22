#pragma once

#include <asio.hpp>
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

namespace qls
{
    /*
    * @brief 基类房间
    */
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

        struct BaseUserSetting
        {
            // 发送函数
            using SendFunction = std::function<asio::awaitable<size_t>(
                std::string_view data, long long, int, int)>;

            long long user_id;
            Equipment equipment = Equipment::Unknown;
            SendFunction sendFunction;
        };

        BaseRoom() = default;
        virtual ~BaseRoom() = default;

        /*
        * @brief 用户连接加入房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true成功 | false失败
        */
        bool baseJoinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user);

        /*
        * @brief 用户连接离开房间
        * @param socket_ptr socket指针
        * @return true成功 | false失败
        */
        bool baseLeaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        /*
        * @brief 发送消息给类中所有用户（广播）
        * @param data 二进制数据
        * @return 协程 true成功 | false失败
        */
        asio::awaitable<bool> baseSendData(const std::string& data);

        /*
        * @brief 发送消息给某用户
        * @param data 二进制数据
        * @param user_id 用户id
        * @return 协程 true成功 | false失败
        */
        asio::awaitable<bool> baseSendData(const std::string& data, long long user_id);

    private:
        std::unordered_map<std::shared_ptr<asio::ip::tcp::socket>,
            BaseUserSetting>                                        m_userMap;
        std::shared_mutex                                           m_userMap_mutex;

        std::queue<std::shared_ptr<asio::ip::tcp::socket>>          m_userDeleteQueue;
        std::shared_mutex                                           m_userDeleteQueue_mutex;
    };
}
