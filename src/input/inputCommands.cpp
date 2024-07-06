#include "inputCommands.h"

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
} // namespace qls

