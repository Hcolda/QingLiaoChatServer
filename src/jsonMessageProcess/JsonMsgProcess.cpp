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
        return makeMessage("error", msg);
    }

    qjson::JObject JsonMessageProcess::makeMessage(const std::string& state, const std::string& msg)
    {
        qjson::JObject json;

        json["state"] = state;
        json["message"] = msg;

        return json;
    }

    qjson::JObject JsonMessageProcess::makeSuccessMessage(const std::string& msg)
    {
        return makeMessage("success", msg);
    }

    qjson::JObject JsonMessageProcess::getUserPublicInfo(long long user_id)
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::hasUser(long long user_id)
    {
        auto returnJson = makeSuccessMessage("Successfully getting result!");
        returnJson["result"] = serverManager.hasUser(user_id);
        return returnJson;
    }

    qjson::JObject JsonMessageProcess::searchUser(const std::string& user_name)
    {
        return makeErrorMessage("This function is imcompleted.");
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
            case 1:
                // register
                return register_user(param["email"].getString(), param["password"].getString());
            case 2:
                // has user
                return hasUser(param["user_id"].getInt());
            case 3:
                // search friend
                return searchUser(param["user_name"].getString());
            case 4:
                // add friend
                return addFriend(param["user_id"].getInt());
            case 5:
                // add group
                return addGroup(param["group_id"].getInt());
            case 6:
                // get friend list
                return getFriendList();
            case 7:
                // get group list
                return getGroupList();
            case 8:
                // send friend message
                return sendFriendMessage(param["user_id"].getInt(), param["message"].getString());
            case 9:
                // send group message
                return sendGroupMessage(param["group_id"].getInt(), param["message"].getString());
            case 10:
                // accept friend verification
                return acceptFriendVerification(param["user_id"].getInt(),
                    param["is_accept"].getBool());
            case 11:
                // get friend verification list
                return getFriendVerificationList();
            case 12:
                // accept group verification
                return acceptGroupVerification(param["group_id"].getInt(),
                    param["user_id"].getInt(), param["is_accept"].getBool());
            case 13:
                // get group verification list
                return getGroupVerificationList();
            default:
                return makeErrorMessage("There isn't a function match your request.");
            }
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
            auto returnJson = makeSuccessMessage("Logining successfully!");
            this->m_user_id = user_id;

            serverLogger.info("用户", user_id, "登录至服务器");

            return returnJson;
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

        auto returnJson = makeSuccessMessage("Successfully create a new user!");
        auto id = ptr->getUserID();
        returnJson["user_id"] = id;

        serverLogger.info("注册了新用户: ", id);

        return returnJson;
    }

    qjson::JObject JsonMessageProcess::addFriend(long long friend_id)
    {
        if (serverManager.getUser(this->m_user_id)->addFriend(friend_id))
        {
            serverLogger.info("用户", (long long)this->m_user_id, "向用户", friend_id, "发送好友申请");
            return makeSuccessMessage("Sucessfully sending application!");
        }
        else return makeErrorMessage("Can't send application");
    }

    qjson::JObject JsonMessageProcess::acceptFriendVerification(long long user_id, bool is_accept)
    {
        if (is_accept)
        {
            serverManager.setFriendVerified(this->m_user_id, user_id, this->m_user_id, true);
            return makeSuccessMessage("Successfully adding a friend!");
        }
        else
        {
            serverManager.removeFriendRoomVerification(this->m_user_id, user_id);
            return makeSuccessMessage("Successfully rejecting a user!");
        }
    }

    qjson::JObject JsonMessageProcess::getFriendList()
    {
        auto set = std::move(serverManager.getUser(this->m_user_id)->getFriendList());
        qjson::JObject returnJson = makeSuccessMessage("Sucessfully getting friend list!");

        for (auto i : set)
        {
            returnJson["friend_list"].push_back(i);
        }

        serverLogger.info("用户", (long long)this->m_user_id, "获取朋友列表");

        return returnJson;
    }

    qjson::JObject JsonMessageProcess::getFriendVerificationList()
    {
        auto map = serverManager.getUser(this->m_user_id)->getFriendVerificationList();
        qjson::JObject localVector;
        for (const auto& [user_id, user_struct] : map)
        {
            qjson::JObject localJson;
            localJson["user_id"] = user_id;
            localJson["verification_type"] = (int)user_struct.verification_type;
            localJson["message"] = user_struct.message;

            localVector.push_back(localJson);
        }

        auto returnJson = makeSuccessMessage("Successfully getting the list!");
        returnJson["result"] = localVector;

        return returnJson;
    }

    qjson::JObject JsonMessageProcess::addGroup(long long group_id)
    {
        if (!serverManager.hasGroupRoom(group_id))
            return makeErrorMessage("There isn't a group room match this id!");

        if (serverManager.getUser(this->m_user_id)->addGroup(group_id))
        {
            serverLogger.info("用户", (long long)this->m_user_id, "向群聊", group_id, "发送申请");
            return makeSuccessMessage("Sucessfully sending an application!");
        }
        else return makeErrorMessage("Can't send an application!");
    }

    qjson::JObject JsonMessageProcess::acceptGroupVerification(long long group_id, long long user_id, bool is_accept)
    {
        if (is_accept)
        {
            serverManager.setGroupRoomGroupVerified(group_id, user_id, true);
            return makeSuccessMessage("Successfully adding a member into the group!");
        }
        else
        {
            serverManager.removeGroupRoomVerification(group_id, user_id);
            return makeSuccessMessage("Successfully rejecting a user!");
        }
    }

    qjson::JObject JsonMessageProcess::getGroupList()
    {
        auto set = std::move(serverManager.getUser(this->m_user_id)->getGroupList());
        qjson::JObject returnJson = makeSuccessMessage("Sucessfully getting group list!");

        for (auto i : set)
        {
            returnJson["friend_list"].push_back(i);
        }

        serverLogger.info("用户", (long long)this->m_user_id, "获取群聊列表");

        return returnJson;
    }

    qjson::JObject JsonMessageProcess::getGroupVerificationList()
    {
        auto map = std::move(serverManager.getUser(this->m_user_id)->getGroupVerificationList());
        auto returnJson = makeSuccessMessage("Successfully getting a list!");
        for (const auto& [group_id, user_struct] : map)
        {
            auto group = std::to_string(group_id);
            returnJson["result"][group.c_str()]["user_id"] = user_struct.user_id;
            returnJson["result"][group.c_str()]["verification_type"] = (int)user_struct.verification_type;
            returnJson["result"][group.c_str()]["message"] = user_struct.message;
        }

        return returnJson;
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

        return makeSuccessMessage("Successfully sending this message!");
    }

    qjson::JObject JsonMessageProcess::sendGroupMessage(long long group_id, const std::string& msg)
    {
        if (!serverManager.getUser(this->m_user_id)->userHasGroup(group_id))
            return makeErrorMessage("You don't has the group!");

        serverManager.getGroupRoom(group_id)->sendMessage(this->m_user_id, msg);
        serverLogger.info("用户", (long long)this->m_user_id, "向群聊", group_id, "发送消息");

        return makeSuccessMessage("Successfully sending this message!");
    }

    const std::multimap<std::string, long long> JsonMessageProcess::m_function_map(
        {
            {"login", 0},
            {"register", 1},
            {"has_user", 2},
            {"search_user", 3},
            {"add_friend", 4},
            {"add_group", 5},
            {"get_friend_list", 6},
            {"get_group_list", 7},
            {"send_friend_message", 8},
            {"send_group_message", 9},
            {"accept_friend_verification", 10},
            {"get_friend_verification_list", 11},
            {"accept_group_verification", 12},
            {"get_group_verification_list", 13}
        }
    );
}
