#ifndef JSON_MESSAGE_PROCESS_H
#define JSON_MESSAGE_PROCESS_H

#include <string>
#include <map>

#include <Json.h>

namespace qls
{
    class JsonMessageProcess
    {
    public:
        JsonMessageProcess(long long user_id);
        ~JsonMessageProcess() = default;

        static qjson::JObject makeErrorMessage(const std::string& msg);
        static qjson::JObject makeMessage(const std::string& state, const std::string& msg);
        static qjson::JObject makeSuccessMessage(const std::string& msg);

        static qjson::JObject getUserPublicInfo(long long user_id);

        static qjson::JObject hasUser(long long user_id);
        static qjson::JObject searchUser(const std::string& user_name);

        long long getLocalUserID() const;
        qjson::JObject processJsonMessage(const qjson::JObject& json);

    protected:
        qjson::JObject login(long long user_id, const std::string& password);
        qjson::JObject login(const std::string& email, const std::string& password);
        
        qjson::JObject register_user(const std::string& email, const std::string& password);
        
        qjson::JObject addFriend(long long friend_id);
        qjson::JObject acceptFriendVerification(long long user_id, bool is_accept);
        qjson::JObject getFriendList();
        qjson::JObject getFriendVerificationList();
        
        qjson::JObject addGroup(long long group_id);
        qjson::JObject acceptGroupVerification(long long group_id, long long user_id, bool is_accept);
        qjson::JObject getGroupList();
        qjson::JObject getGroupVerificationList();

        qjson::JObject sendFriendMessage(long long friend_id, const std::string& msg);
        qjson::JObject sendGroupMessage(long long group_id, const std::string& msg);
        
    private:
        std::atomic<long long> m_user_id;

        static const std::multimap<std::string, long long> m_function_map;
    };
}

#endif // !JSON_MESSAGE_PROCESS_H