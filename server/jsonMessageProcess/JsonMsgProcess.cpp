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
// JsonMessageProcessCommandList
// -----------------------------------------------------------------------------------------------

class JsonMessageProcessCommandList
{
public:
    JsonMessageProcessCommandList() {
        auto init_command = [&](std::string_view function_name, const std::shared_ptr<JsonMessageCommand>& command_ptr) -> bool {
            if (m_function_map.find(function_name) != m_function_map.cend() || !command_ptr)
                return false;

            m_function_map.emplace(function_name, JsonMessageCommandInfomation{command_ptr, command_ptr->getOption()});
            return true;
        };

        init_command("register", std::make_shared<RegisterCommand>());
        init_command("has_user", std::make_shared<HasUserCommand>());
        init_command("search_user", std::make_shared<SearchUserCommand>());
        init_command("add_friend", std::make_shared<AddFriendCommand>());
        init_command("add_group", std::make_shared<AddGroupCommand>());
        init_command("get_friend_list", std::make_shared<GetFriendListCommand>());
        init_command("get_group_list", std::make_shared<GetGroupListCommand>());
        init_command("send_friend_message", std::make_shared<SendFriendMessageCommand>());
        init_command("send_group_message", std::make_shared<SendGroupMessageCommand>());
        init_command("accept_friend_verification", std::make_shared<AcceptFriendVerificationCommand>());
        init_command("get_friend_verification_list", std::make_shared<GetFriendVerificationListCommand>());
        init_command("accept_group_verification", std::make_shared<AcceptGroupVerificationCommand>());
        init_command("get_group_verification_list", std::make_shared<GetGroupVerificationListCommand>());
        init_command("reject_friend_verification", std::make_shared<RejectFriendVerificationCommand>());
        init_command("reject_group_verification", std::make_shared<RejectGroupVerificationCommand>());
        init_command("create_group", std::make_shared<CreateGroupCommand>());
        init_command("remove_group", std::make_shared<RemoveGroupCommand>());
        init_command("leave_group", std::make_shared<LeaveGroupCommand>());
        init_command("remove_friend", std::make_shared<RemoveFriendCommand>());
    }
    ~JsonMessageProcessCommandList() = default;

    bool addCommand(std::string_view function_name, const std::shared_ptr<JsonMessageCommand>& command_ptr);
    bool hasCommand(std::string_view function_name) const;
    std::shared_ptr<JsonMessageCommand> getCommand(std::string_view function_name);
    const std::shared_ptr<JsonMessageCommand> getCommand(std::string_view function_name) const;
    const std::vector<JsonMessageCommand::JsonOption>& getCommandOptions(std::string_view function_name) const;
    bool removeCommand(std::string_view function_name);

private:
    struct JsonMessageCommandInfomation
    {
        std::shared_ptr<JsonMessageCommand> command_ptr;
        std::vector<JsonMessageCommand::JsonOption> json_options;
    };

    std::unordered_map<std::string, JsonMessageCommandInfomation, string_hash, std::equal_to<>>
                                m_function_map;
    mutable std::shared_mutex   m_function_map_mutex;
};

bool JsonMessageProcessCommandList::addCommand(std::string_view function_name, const std::shared_ptr<JsonMessageCommand>& command_ptr)
{
    if (hasCommand(function_name) || !command_ptr)
        return false;
    
    std::unique_lock<std::shared_mutex> unique_lock1(m_function_map_mutex);
    m_function_map.emplace(function_name, JsonMessageCommandInfomation{command_ptr, command_ptr->getOption()});
    return true;
}

bool JsonMessageProcessCommandList::hasCommand(std::string_view function_name) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    return m_function_map.find(function_name) != m_function_map.cend();
}

std::shared_ptr<JsonMessageCommand> JsonMessageProcessCommandList::getCommand(std::string_view function_name)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    auto iter = m_function_map.find(function_name);
    if (iter == m_function_map.cend())
        throw std::system_error(make_error_code(qls_errc::null_pointer));
    return iter->second.command_ptr;
}

const std::shared_ptr<JsonMessageCommand> JsonMessageProcessCommandList::getCommand(std::string_view function_name) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    auto iter = m_function_map.find(function_name);
    if (iter == m_function_map.cend())
        throw std::system_error(make_error_code(qls_errc::null_pointer));
    return iter->second.command_ptr;
}

const std::vector<JsonMessageCommand::JsonOption> &JsonMessageProcessCommandList::getCommandOptions(std::string_view function_name) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_function_map_mutex);
    auto iter = m_function_map.find(function_name);
    if (iter == m_function_map.cend())
        throw std::system_error(make_error_code(qls_errc::null_pointer));
    return iter->second.json_options;
}

bool JsonMessageProcessCommandList::removeCommand(std::string_view function_name)
{
    if (!hasCommand(function_name))
        return false;

    std::unique_lock<std::shared_mutex> unique_lock1(m_function_map_mutex);
    auto [_, info] = *(m_function_map.find(function_name));
    m_function_map.erase(function_name);
    return true;
}

// -----------------------------------------------------------------------------------------------
// JsonMessageProcessImpl
// -----------------------------------------------------------------------------------------------

class JsonMessageProcessImpl
{
public:
    JsonMessageProcessImpl(UserID user_id) :
        m_user_id(user_id) {}

    static qjson::JObject getUserPublicInfo(UserID user_id);

    static qjson::JObject hasUser(UserID user_id);
    static qjson::JObject searchUser(std::string_view user_name);

    UserID getLocalUserID() const;

    qjson::JObject processJsonMessage(const qjson::JObject& json, const SocketService& sf);

    qjson::JObject login(UserID user_id, std::string_view password, std::string_view device, const SocketService& sf);
    qjson::JObject login(std::string_view email, std::string_view password, std::string_view device);

private:
    UserID                      m_user_id;
    mutable std::shared_mutex   m_user_id_mutex;

    static JsonMessageProcessCommandList m_jmpc_list;
};

JsonMessageProcessCommandList JsonMessageProcessImpl::m_jmpc_list;

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

qjson::JObject JsonMessageProcessImpl::processJsonMessage(const qjson::JObject& json, const SocketService& sf)
{
    try {
        serverLogger.debug("Json body: ", qjson::JWriter::fastWrite(json));
        if (json.getType() != qjson::JDict)
            return makeErrorMessage("The data body must be json dictory type!");
        else if (!json.hasMember("function"))
            return makeErrorMessage("\"function\" must be included in json dictory!");
        else if (!json.hasMember("parameters"))
            return makeErrorMessage("\"function\" must be included in json dictory!");
        else if (json["function"].getType() != qjson::JString)
            return makeErrorMessage("\"function\" must be string type!");
        else if (json["parameters"].getType() != qjson::JDict)
            return makeErrorMessage("\"parameters\" must be dictory type!");
        std::string function_name = json["function"].getString();
        qjson::JObject param = json["parameters"];

        // check if user has logined
        {
            std::shared_lock<std::shared_mutex> shared_lock1(m_user_id_mutex);
            // Check if userid == -1
            if (m_user_id == UserID(-1) &&
                function_name != "login" &&
                (!m_jmpc_list.hasCommand(function_name) ||
                    m_jmpc_list.getCommand(function_name)->getCommandType() & JsonMessageCommand::NormalType)) {
                    return makeErrorMessage("You haven't logged in!");
            }
        }

        if (function_name == "login")
            return login(UserID(param["user_id"].getInt()), param["password"].getString(), param["device"].getString(), sf);

        if (!m_jmpc_list.hasCommand(function_name))
            return makeErrorMessage("There isn't a function that matches the name!");
        
        auto command_ptr = m_jmpc_list.getCommand(function_name);
        const qjson::dict_t& param_dict = param.getDict();
        for (const auto& [name, type]: m_jmpc_list.getCommandOptions(function_name)) {
            auto local_iter = param_dict.find(name);
            if (local_iter == param_dict.cend())
                return makeErrorMessage(std::format("Lost a parameter: {}.", name));
            if (local_iter->second.getType() != type)
                return makeErrorMessage(std::format("Wrong parameter type: {}.", name));
        }

        UserID user_id;
        {
            std::shared_lock<std::shared_mutex> id_lock(m_user_id_mutex);
            user_id = m_user_id;
        }
        return command_ptr->execute(user_id, std::move(param));
    } catch (const std::exception& e) {
#ifndef _DEBUG
        return makeErrorMessage("Unknown error occured!");
#else
        return makeErrorMessage(std::string("Unknown error occured: ") + e.what());
#endif
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
