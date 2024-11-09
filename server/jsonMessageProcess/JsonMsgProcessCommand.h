#ifndef JSON_MESSAGE_PROCESS_COMMAND_H
#define JSON_MESSAGE_PROCESS_COMMAND_H

#include <option.hpp>
#include <Json.h>

namespace qls
{

class JsonMessageCommand
{
public:
    enum CommandType : int
    {
        NormalType = 0,
        LoginType = 1, // Unless login function, others had better not use it.
        NeedLoginType = 2
    };

    JsonMessageCommand() = default;
    virtual ~JsonMessageCommand() = default;

    virtual opt::Option getOption() const = 0;
    virtual int getCommandType() const = 0;
    virtual qjson::JObject execute(opt::Option opt, bool& success) = 0;
};

class LoginCommand : public JsonMessageCommand
{
public:
    LoginCommand() = default;
    ~LoginCommand() = default;

    opt::Option getOption() const
    {
        static opt::Option opt;
        opt.add("user_id", opt::Option::OptionType::OPT_REQUIRED);
        opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
        opt.add("device", opt::Option::OptionType::OPT_REQUIRED);
    }

    int getCommandType() const
    {
        return LoginType;
    }

    qjson::JObject execute(opt::Option opt, bool& success);
};

class RegisterCommand : public JsonMessageCommand
{
public:
    RegisterCommand() = default;
    ~RegisterCommand() = default;

    opt::Option getOption() const
    {
        static opt::Option opt;
        opt.add("email", opt::Option::OptionType::OPT_REQUIRED);
        opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
    }

    int getCommandType() const
    {
        return NormalType;
    }

    qjson::JObject execute(opt::Option opt, bool& success);
};

class AddFriendCommand : public JsonMessageCommand
{
public:
    AddFriendCommand() = default;
    ~AddFriendCommand() = default;

    
}

} // namespace qls


#endif // !JSON_MESSAGE_PROCESS_COMMAND_H
