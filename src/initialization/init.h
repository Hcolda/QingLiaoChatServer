#pragma once

#include <iostream>
#include <atomic>
#include <Ini.h>

#include "network.h"
#include "socketFunctions.h"

namespace qls
{
    class Init
    {
    public:
        Init() = default;
        ~Init() = default;

        /*
        * @brief 创建配置文件
        */
        static void createConfig();

        /*
        * @brief 读取配置文件
        */
        static qini::INIObject readConfig();
    };

    int init();
}
