#include "commands.h"

#include <iostream>
#include <userid.hpp>
#include "session.h"

extern qls::Session session;

namespace qls
{

class HelpCommand: public Command
{
public:
    HelpCommand(CommandManager& cm):
        m_command_manager(cm) {}
    ~HelpCommand() = default;

    opt::Option getOption() const
    {
        opt::Option option;
        option.add("name", opt::Option::OptionType::OPT_OPTIONAL);
        return option;
    }

    void execute(const opt::Option& option)
    {
        if (option.has_opt_with_value("name"))
        {
            std::string commandName = option.get_string("name");
            if (!m_command_manager.canFindCommand(commandName))
            {
                std::cout << "Could not find command: " << commandName << '\n';
            }
            m_command_manager.getCommand(commandName)->getOption().show();
            return;
        }

        std::shared_lock<std::shared_mutex> local_shared_lock(m_command_manager.m_command_map_mutex);
        for (const auto& [commandName, command_ptr]: std::as_const(m_command_manager.m_command_map))
        {
            std::cout << commandName << '\n';
            command_ptr->getOption().show();
            std::cout << '\n';
        }
    }

private:
    CommandManager& m_command_manager;
};

class ExitCommand: public Command
{
public:
    ExitCommand() = default;
    ~ExitCommand() = default;

    opt::Option getOption() const
    {
        return {};
    }

    void execute(const opt::Option& option)
    {
        exit(0);
    }
};

class RegisterUserCommand: public Command
{
public:
    RegisterUserCommand() = default;
    ~RegisterUserCommand() = default;

    opt::Option getOption() const
    {
        opt::Option opt;
        opt.add("email", opt::Option::OptionType::OPT_REQUIRED);
        opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
        return opt;
    }

    void execute(const opt::Option& opt)
    {
        qls::UserID user_id;
        if (session.registerUser(opt.get_string("email"), opt.get_string("password"), user_id))
            std::cout << "Successfully created a new user! User id is: " << user_id << '\n';
        else
            std::cout << "Failed to created a new user!\n";
    }
};

class LoginUserCommand: public Command
{
public:
    LoginUserCommand() = default;
    ~LoginUserCommand() = default;

    opt::Option getOption() const
    {
        opt::Option opt;
        opt.add("userid", opt::Option::OptionType::OPT_REQUIRED);
        opt.add("password", opt::Option::OptionType::OPT_REQUIRED);
        return opt;
    }

    void execute(const opt::Option& opt)
    {
        qls::UserID user_id;
        if (session.loginUser(qls::UserID(opt.get_int("userid")), opt.get_string("password")))
            std::cout << "Successfully logined a user!\n";
        else
            std::cout << "Failed to logined a user!\n";
    }
};

CommandManager::CommandManager()
{
    addCommand("exit", std::make_shared<ExitCommand>());
    addCommand("help", std::make_shared<HelpCommand>(*this));
    addCommand("registerUser", std::make_shared<RegisterUserCommand>());
    addCommand("loginUser", std::make_shared<LoginUserCommand>());
}

bool CommandManager::addCommand(const std::string &commandName, const std::shared_ptr<Command> &command_ptr)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_command_map_mutex);
    if (m_command_map.find(commandName) != m_command_map.cend()) {
        return false;
    }
    m_command_map.emplace(commandName, command_ptr);
    return true;
}

bool CommandManager::removeCommand(const std::string &commandName)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_command_map_mutex);
    auto iter = m_command_map.find(commandName);
    if (iter == m_command_map.cend()) {
        return false;
    }
    m_command_map.erase(iter);
    return true;
}

bool CommandManager::canFindCommand(const std::string &commandName) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_command_map_mutex);
    return m_command_map.find(commandName) != m_command_map.cend();
}

std::shared_ptr<Command> CommandManager::getCommand(const std::string &commandName) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_command_map_mutex);
    auto iter = m_command_map.find(commandName);
    if (iter == m_command_map.cend()) {
        throw std::logic_error(commandName + " does not exist");
    }
    return iter->second;
}

} // namespace qls
