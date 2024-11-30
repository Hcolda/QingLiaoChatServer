#include "JsonMsgProcess.h"

#include <format>
#include <unordered_set>
#include <vector>

#include <logger.hpp>
#include "manager.h"
#include "regexMatch.hpp"
#include "returnStateMessage.hpp"
#include "definition.hpp"
#include "groupid.hpp"
#include "userid.hpp"
#include "JsonMsgProcessCommand.h"

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
        m_user_id(user_id)
    {
        addCommand("register", std::make_shared<RegisterCommand>());
        addCommand("has_user", std::make_shared<HasUserCommand>());
        addCommand("search_user", std::make_shared<SearchUserCommand>());
        addCommand("add_friend", std::make_shared<AddFriendCommand>());
        addCommand("add_group", std::make_shared<AddGroupCommand>());
        addCommand("get_friend_list", std::make_shared<GetFriendListCommand>());
        addCommand("get_group_list", std::make_shared<GetGroupListCommand>());
        addCommand("send_friend_message", std::make_shared<SendFriendMessageCommand>());
        addCommand("send_group_message", std::make_shared<SendGroupMessageCommand>());
        addCommand("accept_friend_verification", std::make_shared<AcceptFriendVerificationCommand>());
        addCommand("get_friend_verification_list", std::make_shared<GetFriendVerificationListCommand>());
        addCommand("accept_group_verification", std::make_shared<AcceptGroupVerificationCommand>());
        addCommand("get_group_verification_list", std::make_shared<GetGroupVerificationListCommand>());
        addCommand("reject_friend_verification", std::make_shared<RejectFriendVerificationCommand>());
        addCommand("reject_group_verification", std::make_shared<RejectGroupVerificationCommand>());
        addCommand("create_group", std::make_shared<CreateGroupCommand>());
        addCommand("remove_group", std::make_shared<RemoveGroupCommand>());
        addCommand("leave_group", std::make_shared<LeaveGroupCommand>());
        addCommand("remove_friend", std::make_shared<RemoveFriendCommand>());
    }

    static qjson::JObject getUserPublicInfo(UserID user_id);

    static qjson::JObject hasUser(UserID user_id);
    static qjson::JObject searchUser(std::string_view user_name);

    UserID getLocalUserID() const;

    bool addCommand(std::string_view function_name, const std::shared_ptr<JsonMessageCommand> command_ptr);
    bool hasCommand(std::string_view function_name) const;
    std::shared_ptr<JsonMessageCommand> getCommand(std::string_view function_name) const;
    const std::vector<JsonMessageCommand::JsonOption>& getCommandOptions(std::string_view function_name) const;
    bool removeCommand(std::string_view function_name);

    qjson::JObject processJsonMessage(const qjson::JObject& json, const SocketService& sf);

    qjson::JObject login(UserID user_id, std::string_view password, std::string_view device, const SocketService& sf);
    qjson::JObject login(std::string_view email, std::string_view password, std::string_view device);

private:
    UserID                      m_user_id;
    mutable std::shared_mutex   m_user_id_mutex;

    struct JsonMessageCommandInfomation
    {
        std::shared_ptr<JsonMessageCommand> command_ptr;
        std::vector<JsonMessageCommand::JsonOption> json_options;
    };

    std::unordered_map<std::string, JsonMessageCommandInfomation, string_hash, std::equal_to<>>
                                m_function_map;
    mutable std::shared_mutex   m_function_map_mutex;
    std::unordered_set<std::string, string_hash, std::equal_to<>>
                                m_normal_function_set;
    mutable std::shared_mutex   m_normal_function_set_mutex;
    std::unordered_set<std::string, string_hash, std::equal_to<>>
                                m_login_function_set;
    mutable std::shared_mutex   m_login_function_set_mutex;
};

JsonMessageProcess::JsonMessageProcess(UserID user_id) :
    m_process(std::make_unique<JsonMessageProcessImpl>(user_id)) {}

JsonMessageProcess::~JsonMessageProcess() = default;

qjson::JObject JsonMessageProcessImpl::getUserPublicInfo(UserID user_id)
{
    return qjson::JObject();
}

qjson::JObject JsonMessageProcessImpl::hasUser(UserID user_id)
{
    auto returnJson = makeSuccessMessage("Successfully getting result!");
    returnJson["result"] = serverManager.hasUser(user_id);
    return returnJson;
}

qjson::JObject JsonMessageProcessImpl::searchUser(std::string_view user_name)
{
    return makeErrorMessage("This function is incomplete.");
}

UserID JsonMessageProcessImpl::getLocalUserID() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_id_mutex);
    return this->m_user_id;
}

bool JsonMessageProcessImpl::addCommand(std::string_view function_name, const std::shared_ptr<JsonMessageCommand> command_ptr)
{
    if (hasCommand(function_name) || !command_ptr)
        return false;
    
    std::unique_lock<std::shared_mutex> unique_lock1(m_function_map_mutex, std::defer_lock),
                                        unique_lock2(m_normal_function_set_mutex, std::defer_lock),
                                        unique_lock3(m_login_function_set_mutex, std::defer_lock);
    std::lock(unique_lock1, unique_lock2, unique_lock3);

    m_function_map.emplace(function_name, JsonMessageCommandInfomation{command_ptr, command_ptr->getOption()});
    int function_type = command_ptr->getCommandType();
    if (function_type | JsonMessageCommand::NormalType)
        m_normal_function_set.emplace(function_name);
    else if (function_type | JsonMessageCommand::LoginType)
        m_normal_function_set.emplace(function_name);
    return true;
}

bool JsonMessageProcessImpl::hasCommand(std::string_view function_name) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    return m_function_map.find(function_name) != m_function_map.cend();
}

std::shared_ptr<JsonMessageCommand> JsonMessageProcessImpl::getCommand(std::string_view function_name) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    auto iter = m_function_map.find(function_name);
    if (iter == m_function_map.cend())
        throw std::system_error(make_error_code(qls_errc::null_pointer));
    return iter->second.command_ptr;
}

const std::vector<JsonMessageCommand::JsonOption> &JsonMessageProcessImpl::getCommandOptions(std::string_view function_name) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    auto iter = m_function_map.find(function_name);
    if (iter == m_function_map.cend())
        throw std::system_error(make_error_code(qls_errc::null_pointer));
    return iter->second.json_options;
}

bool JsonMessageProcessImpl::removeCommand(std::string_view function_name)
{
    if (!hasCommand(function_name))
        return false;

    std::unique_lock<std::shared_mutex> unique_lock1(m_function_map_mutex, std::defer_lock),
                                        unique_lock2(m_normal_function_set_mutex, std::defer_lock),
                                        unique_lock3(m_login_function_set_mutex, std::defer_lock);
    std::lock(unique_lock1, unique_lock2, unique_lock3);

    auto [_, info] = *(m_function_map.find(function_name));
    m_function_map.erase(function_name);
    int function_type = info.command_ptr->getCommandType();
    if (function_type | JsonMessageCommand::NormalType)
        m_normal_function_set.erase(function_name);
    else if (function_type | JsonMessageCommand::LoginType)
        m_normal_function_set.erase(function_name);

    return true;
}

qjson::JObject JsonMessageProcessImpl::processJsonMessage(const qjson::JObject& json, const SocketService& sf)
{
    try {
        if (json["function"].getType() != qjson::JString)
            return makeErrorMessage("\"function\" must be string type!");
        if (json["parameters"].getType() != qjson::JDict)
            return makeErrorMessage("\"parameters\" must be dictory type!");
        std::string function_name = json["function"].getString();
        qjson::JObject param = json["parameters"];

        // check if user has logined
        {
            std::shared_lock<std::shared_mutex> shared_lock1(m_user_id_mutex, std::defer_lock),
                shared_lock2(m_normal_function_set_mutex, std::defer_lock);
            std::lock(shared_lock1, shared_lock2);
            // Check if userid == -1
            if (m_user_id == UserID(-1) &&
                function_name != "login" &&
                m_normal_function_set.find(function_name) == m_normal_function_set.cend()) {
                return makeErrorMessage("You haven't logged in!");
            }
        }

        if (function_name == "login")
            return login(UserID(param["user_id"].getInt()), param["password"].getString(), param["device"].getString(), sf);
        
        std::shared_lock<std::shared_mutex> m_function_map_lock(m_function_map_mutex, std::defer_lock),
            id_lock(m_user_id_mutex, std::defer_lock);
        std::lock(m_function_map_mutex, id_lock);
        auto iter = m_function_map.find(function_name);
        if (iter == m_function_map.cend())
            return makeErrorMessage("There isn't a function that matches the name!");
        
        const qjson::dict_t& param_dict = param.getDict();
        for (const auto& [name, type]: std::as_const(iter->second.json_options)) {
            auto local_iter = param_dict.find(name);
            if (local_iter == param_dict.cend())
                return makeErrorMessage(std::format("Lost a parameter: {}.", name));
            if (local_iter->second.getType() != type)
                return makeErrorMessage(std::format("Wrong parameter type: {}.", name));
        }

        return iter->second.command_ptr->execute(m_user_id, std::move(param));
    }
    catch (...) {
        return makeErrorMessage("Unknown error occured!");
    }
}

qjson::JObject JsonMessageProcessImpl::login(UserID user_id, std::string_view password, std::string_view device, const SocketService& sf)
{
    if (!serverManager.hasUser(user_id))
        return makeErrorMessage("The user ID or password is wrong!");
    
    auto user = serverManager.getUser(user_id);
    
    if (user->isUserPassword(password)) {
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

        
        serverLogger.debug("User ", user_id.getOriginValue(), " logged into the server");

        return returnJson;
    }
    else return makeErrorMessage("The user ID or password is wrong!");
}

qjson::JObject JsonMessageProcessImpl::login(std::string_view email, std::string_view password, std::string_view device)
{
    if (!qls::RegexMatch::emailMatch(email))
        return makeErrorMessage("Email is invalid");

    // Not completed
    return makeErrorMessage("This function is incomplete.");
}

// -----------------------------------------------------------------------------------------------
// json process
// -----------------------------------------------------------------------------------------------

UserID JsonMessageProcess::getLocalUserID() const
{
    return m_process->getLocalUserID();
}

asio::awaitable<qjson::JObject> JsonMessageProcess::processJsonMessage(const qjson::JObject& json, const SocketService& sf)
{
    co_return m_process->processJsonMessage(json, sf);
}

} // namespace qls
