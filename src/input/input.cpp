#include "input.h"

#include "inputCommands.h"
#include "Logger.hpp"

// 服务器log系统
extern Log::Logger serverLogger;

#define SET_A_COMMAND(variable, name, arguments) \
    if (variable == #name) \
    { \
        qls::##name##_command c; \
        c.setArguments(arguments); \
        return c.execute(); \
    }

class InputImpl
{
public:
    InputImpl() = default;
    ~InputImpl() = default;

    static bool input(const std::string &command)
    {
        std::string first_word;
        std::string::const_iterator iter = command.cbegin();
        while (iter != command.cend() && *iter == ' ') ++iter;
        while (iter != command.cend() && *iter != ' ') first_word += *(iter++);
        if (first_word.empty()) return true;
        std::string arguments(iter, command.cend());

        SET_A_COMMAND(first_word, stop, arguments);

        serverLogger.warning("没有此指令: ", first_word);
        return true;
    }
};

namespace qls
{
    bool Input::input(const std::string &command)
    {
        return InputImpl::input(command);
    }

} // namespace qls
