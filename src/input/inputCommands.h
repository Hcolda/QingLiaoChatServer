#ifndef INPUT_COMMANDS_H
#define INPUT_COMMANDS_H

#include <string>

namespace qls
{
    class Command
    {
    public:
        Command() = default;
        virtual ~Command() = default;
        virtual void setArguments(const std::string& arguments) {}
        virtual bool execute() { return true; }
    };

    class stop_command: public Command
    {
    public:
        stop_command() = default;
        virtual bool execute();
    };
}

#endif // !INPUT_COMMANDS_H