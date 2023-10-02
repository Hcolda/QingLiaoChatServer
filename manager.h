#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

#include "definition.hpp"
#include "room.h"
#include "SQLProcess.hpp"

namespace qls
{
    class Managaer
    {
    protected:
        Managaer() = default;
        ~Managaer() = default;

    public:
        /*
        * @brief 获取全局管理器
        * @return 管理器
        */
        static Managaer& getGlobalManager();

        /*
        * @brief 设置sql
        * @param process sqlProcess
        */
        void setSQLProcess(const std::shared_ptr<quqisql::SQLDBProcess>& process);

        /*
        * @brief 初始化
        */
        void init();

    private:
        std::shared_ptr<quqisql::SQLDBProcess> m_sqlProcess;


    };
}
