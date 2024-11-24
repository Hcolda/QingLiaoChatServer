#ifndef JSON_MESSAGE_PROCESS_COMMAND_H
#define JSON_MESSAGE_PROCESS_COMMAND_H

#include <initializer_list>
#include <string>
#include <Json.h>

namespace qls
{

class JsonMessageCommand
{
public:
    enum CommandType : int
    {
        NormalType = 0, // Use it if the function do not need to login.
        LoginType = 1, // Unless "login" function, other functions had better not use it.
        NeedLoginType = 2 // Use it if the function need to login.
    };

    struct JsonOption
    {
        std::string name;
        qjson::JValueType jsonValueType;
    };

    JsonMessageCommand() = default;
    virtual ~JsonMessageCommand() = default;

    virtual std::initializer_list<JsonOption> getOption() const = 0;
    virtual int getCommandType() const = 0;
    virtual qjson::JObject execute(qjson::JObject parameters, bool& success) = 0;
};

class LoginCommand : public JsonMessageCommand
{
public:
    LoginCommand() = default;
    ~LoginCommand() = default;

    std::initializer_list<JsonOption> getOption() const
    {
        return {
            {"user_id", qjson::JInt},
            {"password", qjson::JString},
            {"device", qjson::JString}};
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(qjson::JObject parameters, bool& success);
};

class RegisterCommand : public JsonMessageCommand
{
public:
    RegisterCommand() = default;
    ~RegisterCommand() = default;

    std::initializer_list<JsonOption> getOption() const
    {
        return {{"email", qjson::JString},
            {"password", qjson::JString}};
    }

    int getCommandType() const
    {
        return NormalType;
    }

    qjson::JObject execute(qjson::JObject parameters, bool& success);
};

class AddFriendCommand : public JsonMessageCommand
{
public:
    AddFriendCommand() = default;
    ~AddFriendCommand() = default;

    std::initializer_list<JsonOption> getOption() const
    {
        return {{"user_id", qjson::JInt}};
    }

    int getCommandType() const
    {
        return NeedLoginType;
    }

    qjson::JObject execute(qjson::JObject parameters, bool& success);
};

} // namespace qls


#endif // !JSON_MESSAGE_PROCESS_COMMAND_H
