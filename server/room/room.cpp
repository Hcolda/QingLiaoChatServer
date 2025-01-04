#include "room.h"

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>

#include "Json.h"
#include "dataPackage.h"
#include "manager.h"
#include "logger.hpp"
#include "user.h"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls
{

/*
* ------------------------------------------------------------------------
* class BaseRoom
* ------------------------------------------------------------------------
*/

struct BaseRoomImpl
{
    BaseRoomImpl(std::pmr::memory_resource *mr):
        m_user_map(mr) {}

    std::pmr::unordered_map<UserID, std::weak_ptr<User>>
                                m_user_map;
    mutable std::shared_mutex   m_user_map_mutex;
};

void BaseRoomImplDeleter::operator()(BaseRoomImpl* bri) noexcept
{
    memory_resource->deallocate(bri, sizeof(BaseRoomImpl));
}

BaseRoom::BaseRoom(std::pmr::memory_resource *mr):
    m_impl(
        [mr](BaseRoomImpl* bri) {
            new(bri) BaseRoomImpl(mr); return bri;
            } (static_cast<BaseRoomImpl*>(mr->allocate(sizeof(BaseRoomImpl)))),
        {mr}) {}

BaseRoom::~BaseRoom() noexcept = default;

void BaseRoom::joinRoom(UserID user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_map_mutex);
    if (m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend())
        return;

    m_impl->m_user_map.emplace(user_id, serverManager.getUser(user_id));
}

bool BaseRoom::hasUser(UserID user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_map_mutex);
    return m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend();
}

void BaseRoom::leaveRoom(UserID user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_map_mutex);
    auto iter = m_impl->m_user_map.find(user_id);
    if (iter == m_impl->m_user_map.cend())
        return;

    m_impl->m_user_map.erase(iter);
}

void BaseRoom::sendData(std::string_view data)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_map_mutex);

    for (const auto& [user_id, user_ptr]: std::as_const(m_impl->m_user_map)) {
        if (!user_ptr.expired())
            user_ptr.lock()->notifyAll(data);
    }
}

void BaseRoom::sendData(std::string_view data, UserID user_id)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_map_mutex);
    if (m_impl->m_user_map.find(user_id) == m_impl->m_user_map.cend())
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
