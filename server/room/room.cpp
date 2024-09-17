#include "room.h"

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>

#include "Json.h"
#include "dataPackage.h"

namespace qls
{
    
struct BaseRoomImpl
{
    std::unordered_map<UserID, std::shared_ptr<User>>
                            m_userMap;
    std::shared_mutex       m_userMap_mutex;
};

/*
* ------------------------------------------------------------------------
* class BaseRoom
* ------------------------------------------------------------------------
*/

BaseRoom::BaseRoom():
    m_impl(std::make_unique<BaseRoomImpl>())
{}

BaseRoom::~BaseRoom() = default;

bool BaseRoom::joinRoom(UserID user_id, const std::shared_ptr<User>& user_ptr)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_userMap_mutex);
    if (!user_ptr->getUserID() != user_id ||
        m_impl->m_userMap.find(user_id) != m_impl->m_userMap.cend())
        return false;

    m_impl->m_userMap.emplace(user_id, user_ptr);
    return true;
}

bool BaseRoom::hasUser(UserID user_id) const
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_userMap_mutex);
    return m_impl->m_userMap.find(user_id) != m_impl->m_userMap.cend();
}

bool BaseRoom::leaveRoom(UserID user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_userMap_mutex);
    auto iter = m_impl->m_userMap.find(user_id);
    if (iter == m_impl->m_userMap.cend())
        return false;

    m_impl->m_userMap.erase(iter);
    return true;
}

void BaseRoom::sendData(std::string_view data)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_userMap_mutex);

    for (auto& [user_id, user_ptr]: m_impl->m_userMap)
    {
        user_ptr->notifyAll(data);
    }
}

void BaseRoom::sendData(std::string_view data, UserID user_id)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_userMap_mutex);

    m_impl->m_userMap.find(user_id)->second->notifyAll(data);
}

/*
* ------------------------------------------------------------------------
* class TextDataRoom
* ------------------------------------------------------------------------
*/

void TextDataRoom::sendData(std::string_view data)
{
    auto package = DataPackage::makePackage(data);
    package->type = 1;
    BaseRoom::sendData(package->packageToString());
}

void TextDataRoom::sendData(std::string_view data, UserID user_id)
{
    auto package = DataPackage::makePackage(data);
    package->type = 1;
    BaseRoom::sendData(package->packageToString(), user_id);
}

} // namespace qls
