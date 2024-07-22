#include "groupPermission.h"

#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <system_error>

#include "qls_error.h"

namespace qls
{
    void GroupPermission::modifyPermission(const std::string& permissionName, PermissionType type)
    {
        std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);
        m_permission_map[permissionName] = type;
    }

    void GroupPermission::removePermission(const std::string& permissionName)
    {
        std::lock_guard<std::shared_mutex> lg(m_permission_map_mutex);

        // 是否有此权限
        auto itor = m_permission_map.find(permissionName);
        if (itor == m_permission_map.end())
            throw std::system_error(qls_errc::no_permission, "no permission: " + permissionName);

        m_permission_map.erase(itor);
    }

    GroupPermission::PermissionType GroupPermission::getPermissionType(const std::string& permissionName) const
    {
        std::shared_lock<std::shared_mutex> sl(m_permission_map_mutex);

        // 是否有此权限
        auto itor = m_permission_map.find(permissionName);
        if (itor == m_permission_map.end())
            throw std::system_error(qls_errc::no_permission, "no permission: " + permissionName);

        return itor->second;
    }

    std::unordered_map<std::string, GroupPermission::PermissionType> GroupPermission::getPermissionList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_permission_map_mutex);
        return m_permission_map;
    }

    void GroupPermission::modifyUserPermission(long long user_id, PermissionType type)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);
        m_user_permission_map[user_id] = type;
    }

    void GroupPermission::removeUser(long long user_id)
    {
        std::lock_guard<std::shared_mutex> lg(m_user_permission_map_mutex);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::system_error(qls_errc::user_not_existed, "no user: " + std::to_string(user_id));

        m_user_permission_map.erase(itor);
    }

    bool GroupPermission::userHasPermission(long long user_id, const std::string& permissionName) const
    {
        std::shared_lock<std::shared_mutex> sl1(m_permission_map_mutex, std::defer_lock);
        std::shared_lock<std::shared_mutex> sl2(m_user_permission_map_mutex, std::defer_lock);
        // 同时加锁
        std::lock(sl1, sl2);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::system_error(qls_errc::user_not_existed, "no user: " + std::to_string(user_id));

        // 是否有此权限
        auto itor2 = m_permission_map.find(permissionName);
        if (itor2 == m_permission_map.end())
            throw std::system_error(qls_errc::no_permission, "no permission: " + permissionName);

        // 返回权限
        return itor->second >= itor2->second;
    }

    GroupPermission::PermissionType GroupPermission::getUserPermissionType(long long user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);

        // 是否有此user
        auto itor = m_user_permission_map.find(user_id);
        if (itor == m_user_permission_map.end())
            throw std::system_error(qls_errc::user_not_existed, "no user: " + std::to_string(user_id));

        return itor->second;
    }
    std::unordered_map<long long, GroupPermission::PermissionType> GroupPermission::getUserPermissionList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);
        return m_user_permission_map;
    }

    std::vector<long long> GroupPermission::getDefaultUserList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);

        std::vector<long long> return_vector;
        std::for_each(m_user_permission_map.cbegin(), m_user_permission_map.cend(),
            [&return_vector](const std::pair<long long, PermissionType>& p) {
            if (p.second == PermissionType::Default) return_vector.push_back(p.first);
            });

        return return_vector;
    }

    std::vector<long long> GroupPermission::getOperatorList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);

        std::vector<long long> return_vector;
        std::for_each(m_user_permission_map.cbegin(), m_user_permission_map.cend(),
            [&return_vector](const std::pair<long long, PermissionType>& p) {
                if (p.second == PermissionType::Operator) return_vector.push_back(p.first);
            });

        return return_vector;
    }

    std::vector<long long> GroupPermission::getAdministratorList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_permission_map_mutex);

        std::vector<long long> return_vector;
        std::for_each(m_user_permission_map.cbegin(), m_user_permission_map.cend(),
            [&return_vector](const std::pair<long long, PermissionType>& p) {
                if (p.second == PermissionType::Administrator) return_vector.push_back(p.first);
            });

        return return_vector;
    }
}
