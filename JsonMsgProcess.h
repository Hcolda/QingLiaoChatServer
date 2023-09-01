#pragma once

#include <string>
#include <Json.hpp>

namespace qls
{
    class JsonMessageProcess
    {
    public:
        JsonMessageProcess(long long user_id);
        ~JsonMessageProcess() = default;

        /*
        * @brief 返回错误消息
        * @param 错误信息
        * @return json格式错误信息
        */
        static qjson::JObject makeErrorMessage(const std::string& msg);

        /*
        * @brief 获取用户的公开的信息
        * @param user id
        * @return 返回的消息的json类
        */
        static qjson::JObject getUserPublicInfo(long long user_id);

        /*
        * @brief 处理json消息总函数
        * @param json类
        * @return 返回的消息的json类
        */
        qjson::JObject processJsonMessage(const qjson::JObject& json);

    protected:
        /*
        * @brief 添加好友
        * @param friend_id 好友id
        * @return 返回的消息的json类
        */
        qjson::JObject addFriend(long long friend_id);

        /*
        * @brief 获取用户的好友列表
        * @return 返回的消息的json类
        */
        qjson::JObject getFriendList();

        /*
        * @brief 添加群聊
        * @param group_id 群聊id
        * @return 返回的消息的json类
        */
        qjson::JObject addGroup(long long group_id);

        /*
        * @brief 获取用户的群聊列表
        * @return 返回的消息的json类
        */
        qjson::JObject getGroupList();

        /*
        * @brief 对好友发送消息
        * @param friend_id 好友id
        * @param msg 要发送的消息
        * @return 返回的消息的json类
        */
        qjson::JObject sendFriendMessage(long long friend_id, const std::string& msg);

        /*
        * @brief 对群聊发送消息
        * @param group_id 群聊id
        * @param msg 要发送的消息
        * @return 返回的消息的json类
        */
        qjson::JObject sendGroupMessage(long long group_id, const std::string& msg);
        
    private:
        const long long m_user_id;
    };
}