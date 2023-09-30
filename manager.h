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
        static Managaer& getGlobalManager();

        void setSQLProcess(const std::shared_ptr<quqisql::SQLDBProcess> process);
        void init();

    private:
        std::shared_ptr<quqisql::SQLDBProcess> m_sqlProcess;


    };
}
