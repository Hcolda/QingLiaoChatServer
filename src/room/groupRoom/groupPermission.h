#ifndef GROUP_PERMISSION_H
#define GROUP_PERMISSION_H

#include <unordered_map>
#include <shared_mutex>
#include <string>
#include <vector>

namespace qls
{
    class GroupPermission
    {
    public:
        enum class PermissionType
        {
            Default = 0,
            Operator,
            Administrator
        };

        GroupPermission() = default;
        ~GroupPermission() = default;

        void modifyPermission(const std::string& permissionName, PermissionType type = PermissionType::Default);
        void removePermission(const std::string& permissionName);
        PermissionType getPermissionType(const std::string& permissionName) const;
        std::unordered_map<std::string, PermissionType> getPermissionList() const;

        void modifyUserPermission(long long user_id, PermissionType type = PermissionType::Default);
        void removeUser(long long user_id);

        bool userHasPermission(long long user_id, const std::string& permissionName) const;
        PermissionType getUserPermissionType(long long user_id) const;
        std::unordered_map<long long, PermissionType> getUserPermissionList() const;
        std::vector<long long> getDefaultUserList() const;
        std::vector<long long> getOperatorList() const;
        std::vector<long long> getAdministratorList() const;

    private:
        std::unordered_map<std::string, PermissionType> m_permission_map;
        mutable std::shared_mutex                       m_permission_map_mutex;

        std::unordered_map<long long, PermissionType>   m_user_permission_map;
        mutable std::shared_mutex                       m_user_permission_map_mutex;
    };
}

#endif // !GROUP_PERMISSION_H
