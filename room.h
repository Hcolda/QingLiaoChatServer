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
            long long user_id;
            Equipment equipment = Equipment::Unknown;
            char key[32 + 1]{ 0 };
            char iv[16 + 1]{ 0 };
        };

        BaseRoom() = default;
        ~BaseRoom() = default;

        /*
        * @brief 用户连接加入房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true成功 | false失败
        */
        bool joinBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user);

        /*
        * @brief 用户连接离开房间
        * @param socket_ptr socket指针
        * @return true成功 | false失败
        */
        bool leaveBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

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
        std::unordered_map<std::shared_ptr<asio::ip::tcp::socket>, BaseUserSetting>    m_userMap;
        std::queue<std::shared_ptr<asio::ip::tcp::socket>>                      m_userDeleteQueue;
        std::shared_mutex                                                       m_userMap_mutex;
    };

    /*
    * @brief 私聊房间基类
    */
    class BasePrivateRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        BasePrivateRoom(long long user_id_1, long long user_id_2);
        ~BasePrivateRoom() = default;

        /*
        * @brief 用户连接加入私聊房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true 加入成功 | false 加入失败
        */
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);

        /*
        * @brief 用户连接离开私聊房间
        * @param socket_ptr socket指针
        * @return true 离开成功 | false 离开失败
        */
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        /*
        * @brief 广播消息
        * @param message 发送的消息（非二进制数据）
        * @param sender_user_id 发送者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendMessage(const std::string& message, long long sender_user_id);

        /*
        * @brief 广播提示消息
        * @param message 发送的消息（非二进制数据）
        * @param sender_user_id 发送者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendTipMessage(const std::string& message);

    private:
        const long long m_user_id_1, m_user_id_2;
    };

    /*
    * @brief 群聊房间
    */
    class BaseGroupRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        BaseGroupRoom(long long group_id);
        ~BaseGroupRoom() = default;

        /*
        * @brief 添加用户进入群聊
        * @param user_id 用户id
        * @return true 添加成功 | false 添加失败
        */
        bool baseAddMember(long long user_id);

        /*
        * @brief 从群聊中移除用户
        * @param user_id 用户id
        * @return true 移除成功 | false 移除失败
        */
        bool baseRemoveMember(long long user_id);

        /*
        * @brief 将用户连接加入群聊房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true 加入成功 | false 加入失败
        */
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);

        /*
        * @brief 从群聊房间删除用户连接
        * @param socket_ptr socket指针
        * @return true 离开成功 | false 离开失败
        */
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        /*
        * @brief 广播消息
        * @param message 发送的消息（非二进制数据）
        * @param sender_user_id 发送者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendMessage(const std::string& message, long long sender_user_id);

        /*
        * @brief 发送提示消息
        * @param message 发送的消息（非二进制数据）
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendTipMessage(const std::string& message);

        /*
        * @brief 发送提示消息给单独某个用户
        * @param message 发送的消息（非二进制数据）
        * @param receiver_user_id 接收者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendUserTipMessage(const std::string& message, long long receiver_user_id);

    private:
        const long long m_group_id;

        std::unordered_set<long long>   m_user_id_map;
        mutable std::shared_mutex       m_user_id_map_mutex;
    };
}
