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

        bool baseJoinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user);
        bool baseLeaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        asio::awaitable<bool> baseSendData(const std::string& data);
        asio::awaitable<bool> baseSendData(const std::string& data, long long user_id);

    private:
        std::unordered_map<std::shared_ptr<asio::ip::tcp::socket>,
            BaseUserSetting>                                        m_userMap;
        std::shared_mutex                                           m_userMap_mutex;

        std::queue<std::shared_ptr<asio::ip::tcp::socket>>          m_userDeleteQueue;
        std::shared_mutex                                           m_userDeleteQueue_mutex;
    };
}
