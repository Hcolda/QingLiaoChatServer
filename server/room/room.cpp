#include "room.h"

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>

#include "Json.h"
#include "dataPackage.h"
#include "manager.h"
#include "logger.hpp"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls
{

/*
* ------------------------------------------------------------------------
* class BaseRoom
* ------------------------------------------------------------------------
*/

bool BaseRoom::joinRoom(UserID user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(this->m_user_set_mutex);
    if (this->m_user_set.find(user_id) != this->m_user_set.cend())
        return false;

    this->m_user_set.emplace(user_id);
    return true;
}

bool BaseRoom::hasUser(UserID user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(this->m_user_set_mutex);
    return this->m_user_set.find(user_id) != this->m_user_set.cend();
}

bool BaseRoom::leaveRoom(UserID user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(this->m_user_set_mutex);
    auto iter = this->m_user_set.find(user_id);
    if (iter == this->m_user_set.cend())
        return false;

    this->m_user_set.erase(iter);
    return true;
}

void BaseRoom::sendData(std::string_view data)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(this->m_user_set_mutex);

    for (auto i = this->m_user_set.cbegin(); i != this->m_user_set.cend(); ++i) {
        serverManager.getUser(*i)->notifyAll(data);
    }
}

void BaseRoom::sendData(std::string_view data, UserID user_id)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(this->m_user_set_mutex);
    if (this->m_user_set.find(user_id) == this->m_user_set.cend())
        throw std::logic_error("User id not in room.");
    serverManager.getUser(user_id)->notifyAll(data);
}

/*
* ------------------------------------------------------------------------
* class TextDataRoom
* ------------------------------------------------------------------------
*/

void TextDataRoom::sendData(std::string_view data)
{
    auto package = DataPackage::makePackage(data);
    package->type = DataPackage::Text;
    BaseRoom::sendData(package->packageToString());
}

void TextDataRoom::sendData(std::string_view data, UserID user_id)
{
    auto package = DataPackage::makePackage(data);
    package->type = DataPackage::Text;
    BaseRoom::sendData(package->packageToString(), user_id);
}

} // namespace qls
