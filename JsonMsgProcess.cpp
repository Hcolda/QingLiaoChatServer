#include "JsonMsgProcess.h"

#include <format>
#include <unordered_set>

#include <Logger.hpp>
#include "manager.h"
#include "regexMatch.hpp"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls
{
    JsonMessageProcess::JsonMessageProcess(long long user_id) :
        m_user_id(user_id) {}

    qjson::JObject JsonMessageProcess::makeErrorMessage(const std::string& msg)
    {
        qjson::JObject json;

        json["state"] = "error";
        json["message"] = msg;

        return json;
    }

    qjson::JObject JsonMessageProcess::makeMessage(const std::string& state, const std::string& msg)
    {
        qjson::JObject json;

        json["state"] = state;
        json["message"] = msg;

        return json;
    }

    qjson::JObject JsonMessageProcess::getUserPublicInfo(long long user_id)
    {
        return qjson::JObject();
    }

    long long JsonMessageProcess::getLocalUserID() const
    {
        return m_user_id;
    }

    qjson::JObject JsonMessageProcess::processJsonMessage(const qjson::JObject& json)
    {
        try
        {
            std::string function_name = json["function"].getString();
            const qjson::JObject& param = json["parameters"];

            // 判断 userid == -1
            if (m_user_id == -1 &&
                function_name != "login" &&
                function_name != "register")
                return makeErrorMessage("You have't been logined!");
            
            auto itor = m_function_map.find(function_name);
            if (itor == m_function_map.cend())
                return makeErrorMessage("There isn't a function match the name!");

            switch (m_function_map.find(function_name)->second)
            {
            case 0:
                // login
                return login(param["user_id"].getInt(), param["password"].getString());
                break;
            case 1:
                return register_user(param["email"].getString(), param["password"].getString());
            }

            return qjson::JObject();
        }
        catch (const std::exception& e)
        {
            return makeErrorMessage(e.what());
        }
    }

    qjson::JObject JsonMessageProcess::login(long long user_id, const std::string& password)
    {
        if (!serverManager.hasUser(user_id))
            return makeErrorMessage("There isn't a user match the ID of this user!");

        if (serverManager.getUser(user_id)->isUserPassword(password))
        {
            auto rejson = makeMessage("success", "Logining successfully!");
            this->m_user_id = user_id;

            serverLogger.info("用户", user_id, "登录至服务器");

            return rejson;
        }
        else return makeErrorMessage("Password is wrong!");
    }

    qjson::JObject JsonMessageProcess::login(const std::string& email, const std::string& password)
    {
        if (!qls::RegexMatch::emailMatch(email))
            return makeErrorMessage("Email is invalid");

        // 未完善
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::register_user(const std::string& email, const std::string& password)
    {
        if (!qls::RegexMatch::emailMatch(email))
            return makeErrorMessage("Email is invalid");

        auto ptr = serverManager.addNewUser();
        ptr->firstUpdateUserPassword(password);
        ptr->updateUserEmail(email);

        auto rejson = makeMessage("success", "Successfully create a new user!");
        auto id = ptr->getUserID();
        rejson["user_id"] = id;

        serverLogger.info("注册了新用户: ", id);

        return rejson;
    }

    qjson::JObject JsonMessageProcess::addFriend(long long friend_id)
    {
        if (serverManager.getUser(this->m_user_id)->addFriend(friend_id))
        {
            serverLogger.info("用户", (long long)this->m_user_id, "向用户", friend_id, "发送好友申请");
            return makeMessage("success", "Sucessfully sending application!");
        }
        else return makeErrorMessage("Can't send application");
    }

    qjson::JObject JsonMessageProcess::getFriendList()
    {
        auto set = std::move(serverManager.getUser(this->m_user_id)->getFriendList());
        qjson::JObject rejson = makeMessage("success", "Sucessfully getting friend list!");

        for (auto i : set)
        {
            rejson["friend_list"].push_back(i);
        }

        serverLogger.info("用户", (long long)this->m_user_id, "获取朋友列表");

        return rejson;
    }

    qjson::JObject JsonMessageProcess::addGroup(long long group_id)
    {
        if (serverManager.getUser(this->m_user_id)->addGroup(group_id))
        {
            serverLogger.info("用户", (long long)this->m_user_id, "向群聊", group_id, "发送申请");
            return makeMessage("success", "Sucessfully sending application!");
        }
        else return makeErrorMessage("Can't send application");
    }

    qjson::JObject JsonMessageProcess::getGroupList()
    {
        auto set = std::move(serverManager.getUser(this->m_user_id)->getGroupList());
        qjson::JObject rejson = makeMessage("success", "Sucessfully getting group list!");

        for (auto i : set)
        {
            rejson["friend_list"].push_back(i);
        }

        serverLogger.info("用户", (long long)this->m_user_id, "获取群聊列表");

        return rejson;
    }

    qjson::JObject JsonMessageProcess::sendFriendMessage(long long friend_id, const std::string& msg)
    {
        if (!serverManager.getUser(this->m_user_id)->userHasFriend(friend_id))
            return makeErrorMessage("You don't has the friend!");

        // 发送消息
        serverManager.getPrivateRoom(
            serverManager.getPrivateRoomId(
                this->m_user_id, friend_id))->sendMessage(
                    msg, this->m_user_id);

        serverLogger.info("用户", (long long)this->m_user_id, "向朋友", friend_id, "发送消息");

        return makeMessage("success", "Successfully sending this message!");
    }

    qjson::JObject JsonMessageProcess::sendGroupMessage(long long group_id, const std::string& msg)
    {
        if (!serverManager.getUser(this->m_user_id)->userHasGroup(group_id))
            return makeErrorMessage("You don't has the group!");

        serverManager.getGroupRoom(group_id)->sendMessage(this->m_user_id, msg);
        serverLogger.info("用户", (long long)this->m_user_id, "向群聊", group_id, "发送消息");

        return makeMessage("success", "Successfully sending this message!");
    }


    const std::multimap<std::string, long long> JsonMessageProcess::m_function_map(
        {
            {"login", 0},
            {"register", 1}
        }
    );
}
