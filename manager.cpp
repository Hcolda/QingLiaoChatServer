#include "manager.h"

#include <stdexcept>

namespace qls
{
    Managaer& Managaer::getGlobalManager()
    {
        static Managaer localManager;
        return localManager;
    }

    void Managaer::setSQLProcess(const std::shared_ptr<quqisql::SQLDBProcess>& process)
    {
        if (!process)
            throw std::invalid_argument("process is nullptr");

        m_sqlProcess = process;
    }

    void Managaer::init()
    {
        
    }
}
