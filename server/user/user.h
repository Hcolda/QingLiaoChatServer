#ifndef USER_H
#define USER_H

#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <shared_mutex>
#include <string>

#include "groupid.hpp"
#include "userid.hpp"
#include "socket.h"
#include "definition.hpp"
#include "qls_error.h"

namespace qls
{

struct UserVerificationStructure
{
    enum class VerificationType
    {
        Unknown = 0,
        Sent,
        Received
    };

    UserID user_id = UserID(0ll);
    /*
    * @brief 验证类型:
    * @brief {Unknown:  无状态}
    * @brief {Sent:     发出的申请}
    * @brief {Received: 接收的申请}
    */
    VerificationType verification_type = VerificationType::Unknown;
    bool has_message = false;
    std::string message;
};

struct UserImpl;

/**
 * @brief Class representing a User.
 */
class User final
{
public:
    /**
     * @brief Constructor to initialize a User object.
     * @param user_id The ID of the user.
     * @param is_create Flag indicating if user is being created.
     */
    User(UserID user_id, bool is_create);

    User(const User&) = delete; // Copy constructor deleted
    User(User&&) = delete; // Move constructor deleted
    ~User();

    // Methods to get user information

    UserID      getUserID() const;
    std::string getUserName() const;
    long long   getRegisteredTime() const;
    int         getAge() const;
    std::string getUserEmail() const;
    std::string getUserPhone() const;
    std::string getUserProfile() const;
    bool        isUserPassword(std::string_view) const;

    // Methods to update user information

    void updateUserName(std::string_view);
    void updateAge(int);
    void updateUserEmail(std::string_view);
    void updateUserPhone(std::string_view);
    void updateUserProfile(std::string_view);
    void firstUpdateUserPassword(std::string_view new_password);
    void updateUserPassword(std::string_view old_password,
        std::string_view new_password);

    // Methods to get user associated information

    bool userHasFriend(UserID friend_user_id) const;
    bool userHasGroup(GroupID group_id) const;

    std::unordered_set<UserID> getFriendList() const;
    std::unordered_set<GroupID> getGroupList() const;

    /**
     * @brief Updates the friend list with a new set of friends.
     * 
     * @tparam T The type of the container holding the friend list.
     * @tparam Y SFINAE parameter to ensure the type T is an unordered_set of UserID.
     * @param set The new friend list to be updated.
     */
    void updateFriendList(const std::unordered_set<UserID>& set);

    /**
     * @brief Updates the friend list with a new set of friends.
     * 
     * @tparam T The type of the container holding the friend list.
     * @tparam Y SFINAE parameter to ensure the type T is an unordered_set of UserID.
     * @param set The new friend list to be updated.
     */
    void updateFriendList(std::unordered_set<UserID>&& set);

    /**
     * @brief Updates the group list with a new set of groups.
     * 
     * @tparam T The type of the container holding the group list.
     * @tparam Y SFINAE parameter to ensure the type T is an unordered_set of GroupID.
     * @param set The new group list to be updated.
     */
    void updateGroupList(const std::unordered_set<GroupID>& set);

    /**
     * @brief Updates the group list with a new set of groups.
     * 
     * @tparam T The type of the container holding the group list.
     * @tparam Y SFINAE parameter to ensure the type T is an unordered_set of GroupID.
     * @param set The new group list to be updated.
     */
    void updateGroupList(std::unordered_set<GroupID>&& set);

    /**
     * @brief Adds a friend to the user's friend list.
     * @param friend_user_id The ID of the friend to add.
     * @return true if adding friend was successful, false otherwise.
     */
    bool addFriend(UserID friend_user_id);

    /**
     * @brief Adds a friend verification entry.
     * @tparam T Type of UserVerificationStructure.
     * @param friend_user_id The ID of the friend.
     * @param u UserVerificationStructure to add.
     */
    void addFriendVerification(UserID friend_user_id, const UserVerificationStructure& u);

    /**
     * @brief Removes a friend verification entry.
     * @param friend_user_id The ID of the friend to remove verification for.
     */
    void removeFriendVerification(UserID friend_user_id);

    /**
     * @brief Retrieves the list of friend verification entries.
     * @return unordered_map containing friend verification entries.
     */
    std::unordered_map<UserID,
        UserVerificationStructure> getFriendVerificationList() const;

    /**
     * @brief Adds a group to the user's group list.
     * @param group_id The ID of the group to add.
     * @return true if adding group was successful, false otherwise.
     */
    bool addGroup(GroupID group_id);

    /**
     * @brief Adds a group verification entry.
     * @tparam T Type of UserVerificationStructure.
     * @param group_id The ID of the group.
     * @param u UserVerificationStructure to add.
     */
    void addGroupVerification(GroupID group_id, const UserVerificationStructure& u);

    /**
     * @brief Removes a group verification entry.
     * @param group_id The ID of the group to remove verification for.
     * @param user_id The ID of the user to remove verification for.
     */
    void removeGroupVerification(GroupID group_id, UserID user_id);

    /**
     * @brief Retrieves the list of group verification entries.
     * @return multimap containing group verification entries.
     */
    std::multimap<GroupID,
        UserVerificationStructure> getGroupVerificationList() const;

    /**
     * @brief Adds a socket to the user's socket map.
     * @param socket_ptr Pointer to the socket to add.
     * @param type DeviceType associated with the socket.
     */
    void addSocket(const std::shared_ptr<qls::Socket>& socket_ptr, DeviceType type);

    /**
     * @brief Checks if the user has a specific socket.
     * @param socket_ptr Pointer to the socket to check.
     * @return true if user has the socket, false otherwise.
     */
    bool hasSocket(const std::shared_ptr<qls::Socket>& socket_ptr) const;

    /**
     * @brief Modifies the type of a socket in the user's socket map.
     * @param socket_ptr Pointer to the socket to modify.
     * @param type New DeviceType associated with the socket.
     */
    void modifySocketType(const std::shared_ptr<qls::Socket>& socket_ptr, DeviceType type);

    /**
     * @brief Removes a socket from the user's socket map.
     * @param socket_ptr Pointer to the socket to remove.
     */
    void removeSocket(const std::shared_ptr<qls::Socket>& socket_ptr);

    /**
     * @brief Notifies all sockets associated with the user.
     * @param data Data to send in the notification.
     */
    void notifyAll(std::string_view data);

    /**
     * @brief Notifies sockets of a specific DeviceType associated with the user.
     * @param type DeviceType of sockets to notify.
     * @param data Data to send in the notification.
     */
    void notifyWithType(DeviceType type, std::string_view data);

private:
    std::unique_ptr<UserImpl> m_impl;
};

} // namespace qls

#endif // !USER_H
