﻿#ifndef GROUP_PERMISSION_H
#define GROUP_PERMISSION_H

#include <unordered_map>
#include <shared_mutex>
#include <string>
#include <vector>

namespace qls
{
    /**
     * @brief Class representing group permissions.
     */
    class GroupPermission
    {
    public:
        /**
         * @brief Enum defining different types of permissions.
         */
        enum class PermissionType
        {
            Default = 0, ///< Default permission level
            Operator,    ///< Operator permission level
            Administrator ///< Administrator permission level
        };

        GroupPermission() = default;
        ~GroupPermission() = default;

        /**
         * @brief Modifies the permission type for a specific permission.
         * @param permissionName Name of the permission to modify.
         * @param type New permission type.
         */
        void modifyPermission(const std::string& permissionName, PermissionType type = PermissionType::Default);

        /**
         * @brief Removes a permission from the permission list.
         * @param permissionName Name of the permission to remove.
         */
        void removePermission(const std::string& permissionName);

        /**
         * @brief Retrieves the type of a specific permission.
         * @param permissionName Name of the permission.
         * @return PermissionType Type of the permission.
         */
        PermissionType getPermissionType(const std::string& permissionName) const;

        /**
         * @brief Retrieves the entire permission list.
         * @return std::unordered_map<std::string, PermissionType> Map of permissions and their types.
         */
        std::unordered_map<std::string, PermissionType> getPermissionList() const;

        /**
         * @brief Modifies the permission type for a specific user.
         * @param user_id ID of the user.
         * @param type New permission type.
         */
        void modifyUserPermission(long long user_id, PermissionType type = PermissionType::Default);

        /**
         * @brief Removes a user from the user permission list.
         * @param user_id ID of the user to remove.
         */
        void removeUser(long long user_id);

        /**
         * @brief Checks if a user has a specific permission.
         * @param user_id ID of the user.
         * @param permissionName Name of the permission.
         * @return true if user has the permission, false otherwise.
         */
        bool userHasPermission(long long user_id, const std::string& permissionName) const;

        /**
         * @brief Retrieves the permission type for a specific user.
         * @param user_id ID of the user.
         * @return PermissionType Type of permission for the user.
         */
        PermissionType getUserPermissionType(long long user_id) const;

        /**
         * @brief Retrieves the entire user permission list.
         * @return std::unordered_map<long long, PermissionType> Map of users and their permission types.
         */
        std::unordered_map<long long, PermissionType> getUserPermissionList() const;

        /**
         * @brief Retrieves a list of users with default permissions.
         * @return std::vector<long long> List of user IDs.
         */
        std::vector<long long> getDefaultUserList() const;

        /**
         * @brief Retrieves a list of users with operator permissions.
         * @return std::vector<long long> List of user IDs.
         */
        std::vector<long long> getOperatorList() const;

        /**
         * @brief Retrieves a list of users with administrator permissions.
         * @return std::vector<long long> List of user IDs.
         */
        std::vector<long long> getAdministratorList() const;

    private:
        std::unordered_map<std::string, PermissionType> m_permission_map; ///< Map of permissions and their types
        mutable std::shared_mutex                       m_permission_map_mutex; ///< Mutex for thread-safe access to permission map

        std::unordered_map<long long, PermissionType>   m_user_permission_map; ///< Map of users and their permission types
        mutable std::shared_mutex                       m_user_permission_map_mutex; ///< Mutex for thread-safe access to user permission map
    };
}

#endif // !GROUP_PERMISSION_H
