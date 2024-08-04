#include "inputCommands.h"

#include <format>

#include <option.hpp>

#include "network.h"
#include "socketFunctions.h"
#include "init.h"
#include "Ini.h"
#include "manager.h"
#include "SQLProcess.hpp"
#include "Logger.hpp"

// 服务器log系统
extern Log::Logger serverLogger;
// ini配置
extern qini::INIObject serverIni;
// 服务器网络系统
extern qls::Network serverNetwork;
// manager
extern qls::Manager serverManager;

namespace qls
{
    bool stop_command::execute()
    {
        serverNetwork.stop();
        return false;
    }

    CommandInfo stop_command::registerCommand()
    {
        return {{}, "stop server"};
    }

    bool show_user_command::execute()
    {
        std::string show;
        auto list = serverManager.getUserList();
        for (auto i = list.begin(); i != list.end(); i++)
        {
            show += std::format("user id: {}, name: {}\n",
                i->first, i->second->getUserName());
        }
        serverLogger.info("用户表: \n", show);
        return true;
    }

    CommandInfo show_user_command::registerCommand()
    {
        return {{}, "show user's infomation"};
    }
} // namespace qls
