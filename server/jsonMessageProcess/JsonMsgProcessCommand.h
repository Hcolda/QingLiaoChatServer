#ifndef JSON_MESSAGE_PROCESS_COMMAND_H
#define JSON_MESSAGE_PROCESS_COMMAND_H

#include <initializer_list>
#include <string>
#include <Json.h>

#include "userid.hpp"
#include "groupid.hpp"

namespace qls
{

class JsonMessageCommand
{
public:
    enum CommandType : int
    {
        NormalType = 0, // Use it if the function do not need to login.
        LoginType = 1 // Use it if the function need to login.
    };

    struct JsonOption
    {
        std::string name;
        qjson::JValueType jsonValueType;
    };

    JsonMessageCommand() = default;
    virtual ~JsonMessageCommand() = default;

    virtual std::vector<JsonOption> getOption() const = 0;
    virtual int getCommandType() const = 0;
    virtual qjson::JObject execute(UserID executor, qjson::JObject parameters) = 0;
};

class RegisterCommand: public JsonMessageCommand
{
public:
    RegisterCommand() = default;
    ~RegisterCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"email", qjson::JString},
            {"password", qjson::JString}};
    }

    int getCommandType() const
    {
        return NormalType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class HasUserCommand: public JsonMessageCommand
{
public:
    HasUserCommand() = default;
    ~HasUserCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return NormalType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class SearchUserCommand: public JsonMessageCommand
{
public:
    SearchUserCommand() = default;
    ~SearchUserCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return NormalType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AddFriendCommand: public JsonMessageCommand
{
public:
    AddFriendCommand() = default;
    ~AddFriendCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AcceptFriendVerificationCommand: public JsonMessageCommand
{
public:
    AcceptFriendVerificationCommand() = default;
    ~AcceptFriendVerificationCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RejectFriendVerificationCommand: public JsonMessageCommand
{
public:
    RejectFriendVerificationCommand() = default;
    ~RejectFriendVerificationCommand() = default;
    
    std::vector<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetFriendListCommand: public JsonMessageCommand
{
public:
    GetFriendListCommand() = default;
    ~GetFriendListCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetFriendVerificationListCommand: public JsonMessageCommand
{
public:
    GetFriendVerificationListCommand() = default;
    ~GetFriendVerificationListCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RemoveFriendCommand: public JsonMessageCommand
{
public:
    RemoveFriendCommand() = default;
    ~RemoveFriendCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AddGroupCommand: public JsonMessageCommand
{
public:
    AddGroupCommand() = default;
    ~AddGroupCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"group_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class AcceptGroupVerificationCommand: public JsonMessageCommand
{
public:
    AcceptGroupVerificationCommand() = default;
    ~AcceptGroupVerificationCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"group_id", qjson::JInt},
                {"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RejectGroupVerificationCommand: public JsonMessageCommand
{
public:
    RejectGroupVerificationCommand() = default;
    ~RejectGroupVerificationCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"group_id", qjson::JInt},
                {"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetGroupListCommand: public JsonMessageCommand
{
public:
    GetGroupListCommand() = default;
    ~GetGroupListCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class GetGroupVerificationListCommand: public JsonMessageCommand
{
public:
    GetGroupVerificationListCommand() = default;
    ~GetGroupVerificationListCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class CreateGroupCommand: public JsonMessageCommand
{
public:
    CreateGroupCommand() = default;
    ~CreateGroupCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class RemoveGroupCommand: public JsonMessageCommand
{
public:
    RemoveGroupCommand() = default;
    ~RemoveGroupCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"group_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class LeaveGroupCommand: public JsonMessageCommand
{
public:
    LeaveGroupCommand() = default;
    ~LeaveGroupCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"group_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class SendFriendMessageCommand: public JsonMessageCommand
{
public:
    SendFriendMessageCommand() = default;
    ~SendFriendMessageCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"friend_id", qjson::JInt},
                {"message", qjson::JString}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

class SendGroupMessageCommand: public JsonMessageCommand
{
public:
    SendGroupMessageCommand() = default;
    ~SendGroupMessageCommand() = default;

    std::vector<JsonOption> getOption() const
    {
        return {{"group_id", qjson::JInt},
                {"message", qjson::JString}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(UserID executor, qjson::JObject parameters);
};

} // namespace qls


#endif // !JSON_MESSAGE_PROCESS_COMMAND_H
