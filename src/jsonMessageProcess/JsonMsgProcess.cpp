#include "JsonMsgProcess.h"

#include <format>
#include <unordered_set>
#include <Logger.hpp>
#include "manager.h"
#include "regexMatch.hpp"
#include "returnStateMessage.hpp"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls
{
    // -----------------------------------------------------------------------------------------------
    // JsonMessageProcessImpl
    // -----------------------------------------------------------------------------------------------

    class JsonMessageProcessImpl
    {
    public:
        JsonMessageProcessImpl(long long user_id) :
            m_user_id(user_id) {}

        static asio::awaitable<qjson::JObject> getUserPublicInfo(long long user_id);

        static asio::awaitable<qjson::JObject> hasUser(long long user_id);
        static asio::awaitable<qjson::JObject> searchUser(const std::string& user_name);

        asio::awaitable<long long> getLocalUserID() const;

        asio::awaitable<qjson::JObject> processJsonMessage(const qjson::JObject& json);

        asio::awaitable<qjson::JObject> login(long long user_id, const std::string& password);
        asio::awaitable<qjson::JObject> login(const std::string& email, const std::string& password);

        asio::awaitable<qjson::JObject> register_user(const std::string& email, const std::string& password);

        asio::awaitable<qjson::JObject> addFriend(long long friend_id);
        asio::awaitable<qjson::JObject> acceptFriendVerification(long long user_id, bool is_accept);
        asio::awaitable<qjson::JObject> getFriendList();
        asio::awaitable<qjson::JObject> getFriendVerificationList();

        asio::awaitable<qjson::JObject> addGroup(long long group_id);
        asio::awaitable<qjson::JObject> acceptGroupVerification(long long group_id, long long user_id, bool is_accept);
        asio::awaitable<qjson::JObject> getGroupList();
        asio::awaitable<qjson::JObject> getGroupVerificationList();

        asio::awaitable<qjson::JObject> sendFriendMessage(long long friend_id, const std::string& msg);
        asio::awaitable<qjson::JObject> sendGroupMessage(long long group_id, const std::string& msg);

    private:

        std::atomic<long long>  m_user_id;

        static const std::multimap<std::string,
            long long>          m_function_map;
    };

    JsonMessageProcess::JsonMessageProcess(long long user_id) :
        m_process(std::make_shared<JsonMessageProcessImpl>(user_id)) {}

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getUserPublicInfo(long long user_id)
    {
        co_return qjson::JObject();
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::hasUser(long long user_id)
    {
        auto returnJson = makeSuccessMessage("Successfully getting result!");
        returnJson["result"] = serverManager.hasUser(user_id);
        co_return returnJson;
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::searchUser(const std::string& user_name)
    {
        co_return makeErrorMessage("This function is imcompleted.");
    }

    asio::awaitable<long long> JsonMessageProcessImpl::getLocalUserID() const
    {
        co_return static_cast<long long>(this->m_user_id);
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::processJsonMessage(const qjson::JObject& json)
    {
        try
        {
            std::string function_name = json["function"].getString();
            const qjson::JObject& param = json["parameters"];

            // 判断 userid == -1
            if (m_user_id == -1 &&
                function_name != "login" &&
                function_name != "register")
                co_return makeErrorMessage("You have't been logined!");
            
            auto itor = m_function_map.find(function_name);
            if (itor == m_function_map.cend())
                co_return makeErrorMessage("There isn't a function match the name!");

            switch (m_function_map.find(function_name)->second)
            {
            case 0:
                // login
                co_return co_await login(param["user_id"].getInt(), param["password"].getString());
            case 1:
                // register
                co_return co_await register_user(param["email"].getString(), param["password"].getString());
            case 2:
                // has user
                co_return co_await hasUser(param["user_id"].getInt());
            case 3:
                // search friend
                co_return co_await searchUser(param["user_name"].getString());
            case 4:
                // add friend
                co_return co_await addFriend(param["user_id"].getInt());
            case 5:
                // add group
                co_return co_await addGroup(param["group_id"].getInt());
            case 6:
                // get friend list
                co_return co_await getFriendList();
            case 7:
                // get group list
                co_return co_await getGroupList();
            case 8:
                // send friend message
                co_return co_await sendFriendMessage(param["user_id"].getInt(), param["message"].getString());
            case 9:
                // send group message
                co_return co_await sendGroupMessage(param["group_id"].getInt(), param["message"].getString());
            case 10:
                // accept friend verification
                co_return co_await acceptFriendVerification(param["user_id"].getInt(),
                    param["is_accept"].getBool());
            case 11:
                // get friend verification list
                co_return co_await getFriendVerificationList();
            case 12:
                // accept group verification
                co_return co_await acceptGroupVerification(param["group_id"].getInt(),
                    param["user_id"].getInt(), param["is_accept"].getBool());
            case 13:
                // get group verification list
                co_return co_await getGroupVerificationList();
            default:
                co_return makeErrorMessage("There isn't a function match your request.");
            }
        }
        catch (const std::exception& e)
        {
            co_return makeErrorMessage(e.what());
        }
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::login(long long user_id, const std::string& password)
    {
        if (!serverManager.hasUser(user_id))
            co_return makeErrorMessage("There isn't a user match the ID of this user!");

        if (serverManager.getUser(user_id)->isUserPassword(password))
        {
            auto returnJson = makeSuccessMessage("Logining successfully!");
            this->m_user_id = user_id;

            serverLogger.info("用户", user_id, "登录至服务器");

            co_return returnJson;
        }
        else co_return makeErrorMessage("Password is wrong!");
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::login(const std::string& email, const std::string& password)
    {
        if (!qls::RegexMatch::emailMatch(email))
            co_return makeErrorMessage("Email is invalid");

        // 未完善
        co_return qjson::JObject();
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::register_user(const std::string& email, const std::string& password)
    {
        if (!qls::RegexMatch::emailMatch(email))
            co_return makeErrorMessage("Email is invalid");

        auto ptr = serverManager.addNewUser();
        ptr->firstUpdateUserPassword(password);
        ptr->updateUserEmail(email);

        auto returnJson = makeSuccessMessage("Successfully create a new user!");
        auto id = ptr->getUserID();
        returnJson["user_id"] = id;

        serverLogger.info("注册了新用户: ", id);

        co_return returnJson;
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::addFriend(long long friend_id)
    {
        if (serverManager.getUser(this->m_user_id)->addFriend(friend_id))
        {
            serverLogger.info("用户", (long long)this->m_user_id, "向用户", friend_id, "发送好友申请");
            co_return makeSuccessMessage("Sucessfully sending application!");
        }
        else co_return makeErrorMessage("Can't send application");
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::acceptFriendVerification(long long user_id, bool is_accept)
    {
        if (is_accept)
        {
            serverManager.setFriendVerified(this->m_user_id, user_id, this->m_user_id, true);
            co_return makeSuccessMessage("Successfully adding a friend!");
        }
        else
        {
            serverManager.removeFriendRoomVerification(this->m_user_id, user_id);
            co_return makeSuccessMessage("Successfully rejecting a user!");
        }
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getFriendList()
    {
        auto set = std::move(serverManager.getUser(this->m_user_id)->getFriendList());
        qjson::JObject returnJson = makeSuccessMessage("Sucessfully getting friend list!");

        for (auto i : set)
        {
            returnJson["friend_list"].push_back(i);
        }

        serverLogger.info("用户", (long long)this->m_user_id, "获取朋友列表");

        co_return returnJson;
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getFriendVerificationList()
    {
        auto map = serverManager.getUser(this->m_user_id)->getFriendVerificationList();
        qjson::JObject localVector;
        for (const auto& [user_id, user_struct] : map)
        {
            qjson::JObject localJson;
            localJson["user_id"] = user_id;
            localJson["verification_type"] = (int)user_struct.verification_type;
            localJson["message"] = user_struct.message;

            localVector.push_back(std::move(localJson));
        }

        auto returnJson = makeSuccessMessage("Successfully getting the list!");
        returnJson["result"] = localVector;

        co_return returnJson;
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::addGroup(long long group_id)
    {
        if (!serverManager.hasGroupRoom(group_id))
            co_return makeErrorMessage("There isn't a group room match this id!");

        if (serverManager.getUser(this->m_user_id)->addGroup(group_id))
        {
            serverLogger.info("用户", (long long)this->m_user_id, "向群聊", group_id, "发送申请");
            co_return makeSuccessMessage("Sucessfully sending an application!");
        }
        else co_return makeErrorMessage("Can't send an application!");
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::acceptGroupVerification(long long group_id, long long user_id, bool is_accept)
    {
        if (is_accept)
        {
            serverManager.setGroupRoomGroupVerified(group_id, user_id, true);
            co_return makeSuccessMessage("Successfully adding a member into the group!");
        }
        else
        {
            serverManager.removeGroupRoomVerification(group_id, user_id);
            co_return makeSuccessMessage("Successfully rejecting a user!");
        }
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getGroupList()
    {
        auto set = std::move(serverManager.getUser(this->m_user_id)->getGroupList());
        qjson::JObject returnJson = makeSuccessMessage("Sucessfully getting group list!");

        for (auto i : set)
        {
            returnJson["friend_list"].push_back(i);
        }

        serverLogger.info("用户", (long long)this->m_user_id, "获取群聊列表");

        co_return returnJson;
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getGroupVerificationList()
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

        co_return returnJson;
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::sendFriendMessage(long long friend_id, const std::string& msg)
    {
        if (!serverManager.getUser(this->m_user_id)->userHasFriend(friend_id))
            co_return makeErrorMessage("You don't has the friend!");

        // 发送消息
        serverManager.getPrivateRoom(
            serverManager.getPrivateRoomId(
                this->m_user_id, friend_id))->sendMessage(
                    msg, this->m_user_id);

        serverLogger.info("用户", (long long)this->m_user_id, "向朋友", friend_id, "发送消息");

        co_return makeSuccessMessage("Successfully sending this message!");
    }

    asio::awaitable<qjson::JObject> JsonMessageProcessImpl::sendGroupMessage(long long group_id, const std::string& msg)
    {
        if (!serverManager.getUser(this->m_user_id)->userHasGroup(group_id))
            co_return makeErrorMessage("You don't has the group!");

        serverManager.getGroupRoom(group_id)->sendMessage(this->m_user_id, msg);
        serverLogger.info("用户", (long long)this->m_user_id, "向群聊", group_id, "发送消息");

        co_return makeSuccessMessage("Successfully sending this message!");
    }

    const std::multimap<std::string, long long> JsonMessageProcessImpl::m_function_map(
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

    // -----------------------------------------------------------------------------------------------
    // json process
    // -----------------------------------------------------------------------------------------------

    asio::awaitable<long long> JsonMessageProcess::getLocalUserID() const
    {
        co_return co_await m_process->getLocalUserID();
    }

    asio::awaitable<qjson::JObject> JsonMessageProcess::processJsonMessage(const qjson::JObject& json)
    {
        co_return co_await m_process->processJsonMessage(json);
    }
}
