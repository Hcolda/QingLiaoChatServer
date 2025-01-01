#include "dataManager.h"

#include <mariadb/conncpp.hpp>

#include "manager.h"

// manager
extern qls::Manager serverManager;

namespace qls
{
    
void DataManager::init()
{
}

void DataManager::addNewUser(UserID user_id, std::size_t pwd_hash)
{
}

void DataManager::changeUserPassword(UserID user_id, std::size_t pwd_hash)
{
}

bool DataManager::verifyUserPassword(UserID user_id, std::size_t pwd_hash)
{
    return false;
}

} // namespace qls
