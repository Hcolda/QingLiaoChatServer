#ifndef COMMANDS_H
#define COMMANDS_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>

#include <option.hpp>

namespace qls
{

class Command
{
public:
    Command() = default;
    virtual ~Command() = default;

    virtual opt::Option getOption() const = 0;
    virtual void execute(const opt::Option&) = 0;
};

class HelpCommand;
class CommandManager final
{
public:
    CommandManager();
    ~CommandManager() = default;

    bool addCommand(const std::string& commandName, const std::shared_ptr<Command>& command_ptr);
    bool removeCommand(const std::string& commandName);
    bool canFindCommand(const std::string& commandName) const;
    std::shared_ptr<Command> getCommand(const std::string& commandName) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Command>>
                                m_command_map;
    mutable std::shared_mutex   m_command_map_mutex;

    friend class HelpCommand;
};

} // namespace qls

#endif
