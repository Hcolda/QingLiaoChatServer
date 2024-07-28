#include "input.h"

#include <unordered_map>
#include <string>
#include <cstring>

#include "inputCommands.h"
#include "Logger.hpp"

// 服务器log系统
extern Log::Logger serverLogger;

using namespace qls;

template<size_t N, size_t N2>
constexpr static void getTargetName(const char(&data)[N], char(&out)[N2])
{
    static_assert(std::strlen(data) < N2);
    size_t size = std::strlen(data);
    for (auto i = 0ull; i < size; ++i)
    {
        if (data[i] == '_')
            out[i] = ' ';
        else
            out[i] = data[i];
    }
}

#define MERGE(x, y) x##y

#define SET_A_COMMAND(name) \
    { \
        char local[sizeof(#name) + 1] {0}; \
        getTargetName(#name, local); \
        m_command_map.emplace(local , MERGE(name, _command){}); \
    }

class InputImpl
{
public:
    InputImpl()
    {
        SET_A_COMMAND(stop);
        SET_A_COMMAND(show_user);
        SET_A_COMMAND(help);
    }

    ~InputImpl() = default;

    bool input(const std::string &command)
    {
        std::string first_word;
        std::string::const_iterator iter = command.cbegin();
        while (iter != command.cend() && *iter == ' ') ++iter;
        while (iter != command.cend() && *iter != '-') first_word += *(iter++);

        first_word = strip(first_word);
        if (first_word.empty()) return true;
        std::string arguments(iter, command.cend());

        auto map_iter = m_command_map.find(first_word);
        if (map_iter == m_command_map.cend())
        {
            serverLogger.warning("没有此指令: ", first_word);
            return true;
        }

        map_iter->second.setArguments(arguments);
        return map_iter->second.execute();

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

    std::unordered_map<std::string, qls::Command> m_command_map;
};

void Input::init()
{
    m_impl = std::make_shared<InputImpl>();
}

bool Input::input(const std::string &command)
{
    return m_impl->input(command);
}
