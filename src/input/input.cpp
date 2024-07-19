#include "input.h"

#include <string_view>

#include "inputCommands.h"
#include "Logger.hpp"

// 服务器log系统
extern Log::Logger serverLogger;

static std::string getTargetName(std::string_view data)
{
    std::string local(data);
    std::replace(local.begin(), local.end(), '_', ' ');
    return local;
}

#define SET_A_COMMAND(variable, name, arguments) \
    if (variable == getTargetName(#name)) \
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
        while (iter != command.cend() && *iter != '-') first_word += *(iter++);

        first_word = strip(first_word);
        if (first_word.empty()) return true;
        std::string arguments(iter, command.cend());

        SET_A_COMMAND(first_word, stop, arguments);
        SET_A_COMMAND(first_word, show_user, arguments);
        SET_A_COMMAND(first_word, help, arguments);

        serverLogger.warning("没有此指令: ", first_word);
        return true;
    }

private:
    static std::string strip(std::string_view data)
    {
        std::string_view::const_iterator first = data.cbegin();
        auto last = data.crbegin();

        while (first != data.cend() && *first == ' ') ++first;
        while (last != data.crend() && *last == ' ') ++last;

        if (first >= last.base()) return {};
        return { first, last.base() };
    }
};

namespace qls
{
    bool Input::input(const std::string &command)
    {
        return InputImpl::input(command);
    }

} // namespace qls
