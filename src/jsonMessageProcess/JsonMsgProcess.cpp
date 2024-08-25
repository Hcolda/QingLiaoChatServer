#include "JsonMsgProcess.h"

#include <format>
#include <unordered_set>
#include <Logger.hpp>
#include "manager.h"
#include "regexMatch.hpp"
#include "returnStateMessage.hpp"
#include "definition.hpp"
#include "groupid.hpp"
#include "userid.hpp"

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
    JsonMessageProcessImpl(UserID user_id) :
        m_user_id(user_id) {}

    static asio::awaitable<qjson::JObject> getUserPublicInfo(UserID user_id);

    static asio::awaitable<qjson::JObject> hasUser(UserID user_id);
    static asio::awaitable<qjson::JObject> searchUser(const std::string& user_name);

    UserID getLocalUserID() const;

    asio::awaitable<qjson::JObject> processJsonMessage(const qjson::JObject& json, const SocketService& sf);

    asio::awaitable<qjson::JObject> login(UserID user_id, const std::string& password, const std::string& device, const SocketService& sf);
    asio::awaitable<qjson::JObject> login(const std::string& email, const std::string& password, const std::string& device);

    asio::awaitable<qjson::JObject> registerUser(const std::string& email, const std::string& password);

    asio::awaitable<qjson::JObject> addFriend(UserID friend_id);
    asio::awaitable<qjson::JObject> acceptFriendVerification(UserID user_id);
    asio::awaitable<qjson::JObject> getFriendList();
    asio::awaitable<qjson::JObject> getFriendVerificationList();

    asio::awaitable<qjson::JObject> addGroup(GroupID group_id);
    asio::awaitable<qjson::JObject> acceptGroupVerification(GroupID group_id, UserID user_id);
    asio::awaitable<qjson::JObject> getGroupList();
    asio::awaitable<qjson::JObject> getGroupVerificationList();

    asio::awaitable<qjson::JObject> sendFriendMessage(UserID friend_id, const std::string& msg);
    asio::awaitable<qjson::JObject> sendGroupMessage(GroupID group_id, const std::string& msg);

private:

    UserID m_user_id;
    mutable std::shared_mutex m_user_id_mutex;

    static const std::multimap<std::string, long long> m_function_map;
};

JsonMessageProcess::JsonMessageProcess(UserID user_id) :
    m_process(std::make_unique<JsonMessageProcessImpl>(user_id)) {}

JsonMessageProcess::~JsonMessageProcess() = default;

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getUserPublicInfo(UserID user_id)
{
    co_return qjson::JObject();
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::hasUser(UserID user_id)
{
    auto returnJson = makeSuccessMessage("Successfully getting result!");
    returnJson["result"] = serverManager.hasUser(user_id);
    co_return returnJson;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::searchUser(const std::string& user_name)
{
    co_return makeErrorMessage("This function is incomplete.");
}

UserID JsonMessageProcessImpl::getLocalUserID() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    return this->m_user_id;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::processJsonMessage(const qjson::JObject& json, const SocketService& sf)
{
    try
    {
        std::string function_name = json["function"].getString();
        const qjson::JObject& param = json["parameters"];

        {
            std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
            // Check if userid == -1
            if (m_user_id == UserID(-1) &&
                function_name != "login" &&
                function_name != "register")
                co_return makeErrorMessage("You haven't logged in!");
        }
        
        auto itor = m_function_map.find(function_name);
        if (itor == m_function_map.cend())
            co_return makeErrorMessage("There isn't a function that matches the name!");

        switch (m_function_map.find(function_name)->second)
        {
        case 0:
            // login
            co_return co_await login(UserID(param["user_id"].getInt()), param["password"].getString(), param["device"].getString(), sf);
        case 1:
            // register
            co_return co_await registerUser(param["email"].getString(), param["password"].getString());
        case 2:
            // has user
            co_return co_await hasUser(UserID(param["user_id"].getInt()));
        case 3:
            // search friend
            co_return co_await searchUser(param["user_name"].getString());
        case 4:
            // add friend
            co_return co_await addFriend(UserID(param["user_id"].getInt()));
        case 5:
            // add group
            co_return co_await addGroup(GroupID(param["group_id"].getInt()));
        case 6:
            // get friend list
            co_return co_await getFriendList();
        case 7:
            // get group list
            co_return co_await getGroupList();
        case 8:
            // send friend message
            co_return co_await sendFriendMessage(UserID(param["user_id"].getInt()), param["message"].getString());
        case 9:
            // send group message
            co_return co_await sendGroupMessage(GroupID(param["group_id"].getInt()), param["message"].getString());
        case 10:
            // accept friend verification
            co_return co_await acceptFriendVerification(UserID(param["user_id"].getInt()));
        case 11:
            // get friend verification list
            co_return co_await getFriendVerificationList();
        case 12:
            // accept group verification
            co_return co_await acceptGroupVerification(GroupID(param["group_id"].getInt()),
                UserID(param["user_id"].getInt()));
        case 13:
            // get group verification list
            co_return co_await getGroupVerificationList();
        default:
            co_return makeErrorMessage("There isn't a function that matches your request.");
        }
    }
    catch (const std::exception& e)
    {
        co_return makeErrorMessage(e.what());
    }
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::login(UserID user_id, const std::string& password, const std::string& device, const SocketService& sf)
{
    if (!serverManager.hasUser(user_id))
        co_return makeErrorMessage("The user ID or password is wrong!");
    
    auto user = serverManager.getUser(user_id);
    
    if (user->isUserPassword(password))
    {
        // check device type
        if (device == "PersonalComputer")
            serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::PersonalComputer);
        else if (device == "Phone")
            serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::Phone);
        else if (device == "Web")
            serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::Web);
        else
            serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::Unknown);

        auto returnJson = makeSuccessMessage("Successfully logged in!");
        std::unique_lock<std::shared_mutex> local_unique_lock(m_user_id_mutex);
        this->m_user_id = user_id;

        
        serverLogger.info("User", user_id.getOriginValue(), "logged into the server");

        co_return returnJson;
    }
    else co_return makeErrorMessage("The user ID or password is wrong!");
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::login(const std::string& email, const std::string& password, const std::string& device)
{
    if (!qls::RegexMatch::emailMatch(email))
        co_return makeErrorMessage("Email is invalid");

    // Not completed
    co_return qjson::JObject();
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::registerUser(const std::string& email, const std::string& password)
{
    if (!qls::RegexMatch::emailMatch(email))
        co_return makeErrorMessage("Email is invalid");

    auto ptr = serverManager.addNewUser();
    ptr->firstUpdateUserPassword(password);
    ptr->updateUserEmail(email);

    qjson::JObject returnJson = makeSuccessMessage("Successfully created a new user!");
    UserID id = ptr->getUserID();
    returnJson["user_id"] = id.getOriginValue();

    serverLogger.info("Registered new user: ", id.getOriginValue());

    co_return returnJson;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::addFriend(UserID friend_id)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    if (serverManager.getUser(this->m_user_id)->addFriend(friend_id))
    {
        serverLogger.info("User", this->m_user_id.getOriginValue(), "sent a friend request to user", friend_id.getOriginValue());
        co_return makeSuccessMessage("Successfully sent application!");
    }
    else co_return makeErrorMessage("Can't send application");
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::acceptFriendVerification(UserID user_id)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    serverManager.getServerVerificationManager().setFriendVerified(this->m_user_id, user_id, this->m_user_id);
        co_return makeSuccessMessage("Successfully added a friend!");
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getFriendList()
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    auto set = std::move(serverManager.getUser(this->m_user_id)->getFriendList());
    qjson::JObject returnJson = makeSuccessMessage("Successfully obtained friend list!");

    for (auto i : set)
    {
        returnJson["friend_list"].push_back(i.getOriginValue());
    }

    serverLogger.info("User ", this->m_user_id.getOriginValue(), " get friend list");

    co_return returnJson;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getFriendVerificationList()
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    auto map = serverManager.getUser(this->m_user_id)->getFriendVerificationList();
    qjson::JObject localVector;
    for (const auto& [user_id, user_struct] : map)
    {
        qjson::JObject localJson;
        localJson["user_id"] = user_id.getOriginValue();
        localJson["verification_type"] = (int)user_struct.verification_type;
        localJson["message"] = user_struct.message;

        localVector.push_back(std::move(localJson));
    }

    auto returnJson = makeSuccessMessage("Successfully obtained verification list!");
    returnJson["result"] = localVector;

    co_return returnJson;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::addGroup(GroupID group_id)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    if (!serverManager.hasGroupRoom(group_id))
        co_return makeErrorMessage("There isn't a group room match this id!");

    if (serverManager.getUser(this->m_user_id)->addGroup(group_id))
    {
        serverLogger.info("User", this->m_user_id.getOriginValue(), "sent a group request to group", group_id.getOriginValue());
        co_return makeSuccessMessage("Successfully sent application!");
    }
    else co_return makeErrorMessage("Can't send application");
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::acceptGroupVerification(GroupID group_id, UserID user_id)
{
    serverManager.getServerVerificationManager().setGroupRoomGroupVerified(group_id, user_id);
        co_return makeSuccessMessage("Successfully added a group!");
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getGroupList()
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    auto set = std::move(serverManager.getUser(this->m_user_id)->getGroupList());
    qjson::JObject returnJson = makeSuccessMessage("Successfully obtained group list!");

    for (auto i : set)
    {
        returnJson["friend_list"].push_back(i.getOriginValue());
    }

    serverLogger.info("User", this->m_user_id.getOriginValue(), "obtained group list");

    co_return returnJson;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::getGroupVerificationList()
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    auto map = std::move(serverManager.getUser(this->m_user_id)->getGroupVerificationList());
    auto returnJson = makeSuccessMessage("Successfully obtained verification list!");
    for (const auto& [group_id, user_struct] : map)
    {
        auto group = std::to_string(group_id.getOriginValue());
        returnJson["result"][group.c_str()]["user_id"] = user_struct.user_id.getOriginValue();
        returnJson["result"][group.c_str()]["verification_type"] = (int)user_struct.verification_type;
        returnJson["result"][group.c_str()]["message"] = user_struct.message;
    }

    co_return returnJson;
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::sendFriendMessage(UserID friend_id, const std::string& msg)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    if (!serverManager.getUser(this->m_user_id)->userHasFriend(friend_id))
        co_return makeErrorMessage("You don't have this friend!");

    // sending a message
    serverManager.getPrivateRoom(
            serverManager.getPrivateRoomId(
                this->m_user_id, friend_id))->sendMessage(msg, this->m_user_id);

    serverLogger.info("User", this->m_user_id.getOriginValue(), "sent a message to user", friend_id.getOriginValue());

    co_return makeSuccessMessage("Successfully sent a message!");
}

asio::awaitable<qjson::JObject> JsonMessageProcessImpl::sendGroupMessage(GroupID group_id, const std::string& msg)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    if (!serverManager.getUser(this->m_user_id)->userHasGroup(group_id))
        co_return makeErrorMessage("You don't have this group!");

    serverManager.getGroupRoom(group_id)->sendMessage(this->m_user_id, msg);
    serverLogger.info("User", this->m_user_id.getOriginValue(), "sent a message to group", group_id.getOriginValue());

    co_return makeSuccessMessage("Successfully sent a message!");
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

UserID JsonMessageProcess::getLocalUserID() const
{
    return this->m_process->getLocalUserID();
}

asio::awaitable<qjson::JObject> JsonMessageProcess::processJsonMessage(const qjson::JObject& json, const SocketService& sf)
{
    co_return co_await this->m_process->processJsonMessage(json, sf);
}

} // namespace qls
