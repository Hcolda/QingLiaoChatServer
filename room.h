#pragma once

#include <asio.hpp>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <chrono>
#include "network.h"

namespace room
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

        enum class UserType
        {
            Default,
            Operator
        };

        struct BaseUserSetting
        {
            long long user_id;
            Equipment equipment = Equipment::Unknown;
            char key[32 + 1]{ 0 };
            char iv[16 + 1]{ 0 };
        };

        class BaseUserInfo
        {
            long long user_id;
            UserType  user_type = UserType::Default;
        };

        BaseRoom() = default;
        ~BaseRoom() = default;

        /*
        * @brief 加入房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true成功 | false失败
        */
        bool joinBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const BaseUserSetting& user);
        
        /*
        * @brief 离开房间
        * @param socket_ptr socket指针
        * @return true成功 | false失败
        */
        bool leaveBaseRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);
        
        /*
        * @brief 禁言用户
        * @brief 
        */
        bool baseMuteMember(long long user_id, const std::chrono::minutes& minutes);

        bool baseUnmuteMember(long long user_id);

        /*
        * @brief 发送消息给类中所有用户（广播）
        * @param data 二进制数据
        * @return 协程 true成功 | false失败
        */
        asio::awaitable<bool> baseSendData(const std::string& data);

    private:
        std::unordered_map<std::shared_ptr<asio::ip::tcp::socket>, BaseUserSetting>    m_userMap;
        std::queue<std::shared_ptr<asio::ip::tcp::socket>>                      m_userDeleteQueue;
        std::shared_mutex                                                       m_userMap_mutex;
    };
}

namespace qls
{
    /*
    * @brief 私聊房间基类
    */
    class BasePrivateRoom : public room::BaseRoom
    {
    public:
        struct User : public room::BaseRoom::BaseUserSetting {};

        BasePrivateRoom(long long user_id_1, long long user_id_2);
        ~BasePrivateRoom() = default;

        /*
        * @brief 加入私聊房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true 加入成功 | false 加入失败
        */
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);

        /*
        * @brief 离开私聊房间
        * @param socket_ptr socket指针
        * @return true 离开成功 | false 离开失败
        */
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        /*
        * @brief 广播消息
        * @param message 发送的消息（非二进制数据）
        * @param user_id 发送者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendData(const std::string& message, long long user_id);

    private:
        const long long m_user_id_1, m_user_id_2;
    };

    /*
    * @brief 群聊房间
    */
    class BaseGroupRoom : public room::BaseRoom
    {
    public:
        struct User : public room::BaseRoom::BaseUserSetting {};

        BaseGroupRoom() = default;
        ~BaseGroupRoom() = default;

        bool baseAddMember(long long user_id);

        bool baseRemoveMember(long long user_id);

        /*
        * @brief 加入私聊房间
        * @param socket_ptr socket指针
        * @param user 用户数据
        * @return true 加入成功 | false 加入失败
        */
        bool joinRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr, const User& user);

        /*
        * @brief 离开私聊房间
        * @param socket_ptr socket指针
        * @return true 离开成功 | false 离开失败
        */
        bool leaveRoom(const std::shared_ptr<asio::ip::tcp::socket>& socket_ptr);

        /*
        * @brief 广播消息
        * @param message 发送的消息（非二进制数据）
        * @param user_id 发送者user_id
        * @return true 发送成功 | false 发送失败
        */
        asio::awaitable<bool> sendData(const std::string& message, long long user_id);

    private:
    };
}


