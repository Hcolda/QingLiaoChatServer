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

    JsonMessageCommand() = default;
    virtual ~JsonMessageCommand() = default;

    virtual const std::initializer_list<std::pair<std::string, qjson::JValueType>>& getOption() const = 0;
    virtual int getCommandType() const = 0;
    virtual qjson::JObject execute(qjson::JObject parameters, bool& success) = 0;
};

class LoginCommand : public JsonMessageCommand
{
public:
    LoginCommand() = default;
    ~LoginCommand() = default;

    const std::initializer_list<std::pair<std::string, qjson::JValueType>>& getOption() const
    {
        static std::initializer_list<std::pair<std::string, qjson::JValueType>> init_list{
            {"user_id", qjson::JInt},
            {"password", qjson::JString},
            {"device", qjson::JString}};
        return init_list;
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

    const std::initializer_list<std::pair<std::string, qjson::JValueType>>& getOption() const
    {
        static std::initializer_list<std::pair<std::string, qjson::JValueType>> init_list{
            {"email", qjson::JString},
            {"password", qjson::JString}};
        return init_list;
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

    const std::initializer_list<std::pair<std::string, qjson::JValueType>>& getOption() const
    {
        static std::initializer_list<std::pair<std::string, qjson::JValueType>> init_list{
            {"user_id", qjson::JInt}};
        return init_list;
    }

    int getCommandType() const
    {
        return NeedLoginType;
    }

    qjson::JObject execute(qjson::JObject parameters, bool& success);
};

} // namespace qls


#endif // !JSON_MESSAGE_PROCESS_COMMAND_H
