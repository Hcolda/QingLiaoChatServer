#ifndef PRIVATE_ROOM_H
#define PRIVATE_ROOM_H

#include "room.h"

namespace qls
{
    /*
    * @brief 私聊房间基类
    */
    class PrivateRoom : public qls::BaseRoom
    {
    public:
        struct User : public qls::BaseRoom::BaseUserSetting {};

        PrivateRoom(long long user_id_1, long long user_id_2);
        ~PrivateRoom() = default;

        /*
        * @brief 初始化
        */
        void init();

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

        /*
        * @brief 获取用户1ID
        * @return 用户1ID
        */
        long long getUserID1() const;

        /*
        * @brief 获取用户2ID
        * @return 用户2ID
        */
        long long getUserID2() const;

        /*
        * @brief 是否有此用户
        * @param user_id 用户的id
        * @return true 有 | false 无
        */
        bool hasUser(long long user_id) const;

    private:
        const long long m_user_id_1, m_user_id_2;

        std::priority_queue<std::pair<long long, std::string>,
            std::vector<std::pair<long long, std::string>>,
            std::greater<std::pair<long long, std::string>>>        m_message_queue;
        std::shared_mutex                                           m_message_queue_mutex;
    };
}

#endif // !PRIVATE_ROOM_H
