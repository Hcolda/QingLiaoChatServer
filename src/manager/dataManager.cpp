#include "dataManager.h"

#include <conncpp.hpp>

#include "manager.h"

// manager
extern qls::Manager serverManager;

namespace qls
{
void DataManager::init()
{
}

void DataManager::addNewUser(long long user_id, size_t pwd_hash)
{
}

void DataManager::changeUserPassword(long long user_id, size_t pwd_hash)
{
}

bool DataManager::verifyUserPassword(long long user_id, size_t pwd_hash)
{
    return false;
}
}
