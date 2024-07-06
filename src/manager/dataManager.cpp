#include "dataManager.h"

#include <conncpp.hpp>

namespace qls
{
    void DataManager::setSQLDBProcess(std::shared_ptr<quqisql::SQLDBProcess> process)
    {
        if (!process) throw std::logic_error("Null process");
        m_sql_process = std::move(process);
    }

    void DataManager::addNewUser(long long user_id, size_t pwd_hash)
    {
        if (!m_sql_process) throw std::logic_error("Null process");

    }

    void DataManager::changeUserPassword(long long user_id, size_t pwd_hash)
    {
        if (!m_sql_process) throw std::logic_error("Null process");

    }

    bool DataManager::verifyUserPassword(long long user_id, size_t pwd_hash)
    {
        if (!m_sql_process) throw std::logic_error("Null process");
        
        return false;
    }
}
