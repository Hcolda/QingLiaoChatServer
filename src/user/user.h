#ifndef USER_H
#define USER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <shared_mutex>
#include <string>

#include "Socket.h"
#include "userStructure.h"

namespace qls
{
    /**
     * @brief Class representing a User.
     */
    class User
    {
    public:
        /**
         * @brief Constructor to initialize a User object.
         * @param user_id The ID of the user.
         * @param is_create Flag indicating if user is being created.
         */
        User(long long user_id, bool is_create);

        User(const User&) = delete; // Copy constructor deleted
        User(User&&) = delete; // Move constructor deleted
        ~User() = default;

        // Methods to get user information

        long long   getUserID() const;
        std::string getUserName() const;
        long long   getRegisteredTime() const;
        int         getAge() const;
        std::string getUserEmail() const;
        std::string getUserPhone() const;
        std::string getUserProfile() const;
        bool        isUserPassword(const std::string&) const;

        // Methods to update user information

        void updateUserName(const std::string&);
        void updateAge(int);
        void updateUserEmail(const std::string&);
        void updateUserPhone(const std::string&);
        void updateUserProfile(const std::string&);
        void firstUpdateUserPassword(const std::string& new_password);
        void updateUserPassword(const std::string& old_password,
            const std::string& new_password);

        // Methods to get user associated information

        bool userHasFriend(long long friend_user_id) const;
        bool userHasGroup(long long group_id) const;

        std::unordered_set<long long> getFriendList() const;
        std::unordered_set<long long> getGroupList() const;

        void updateFriendList(std::unordered_set<long long>);
        void updateGroupList(std::unordered_set<long long>);

        /**
         * @brief Adds a friend to the user's friend list.
         * @param friend_user_id The ID of the friend to add.
         * @return true if adding friend was successful, false otherwise.
         */
        bool addFriend(long long friend_user_id);

        /**
         * @brief Adds a friend verification entry.
         * @tparam T Type of UserVerificationStruct.
         * @param friend_user_id The ID of the friend.
         * @param u UserVerificationStruct to add.
         */
        template<class T, class Y =
            std::enable_if_t<std::is_same_v<
            std::remove_const_t<std::remove_reference_t<T>>,
            UserVerificationStruct>>>
        void addFriendVerification(long long friend_user_id, T&& u)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_friend_verification_map_mutex);
            m_user_friend_verification_map.emplace(friend_user_id, std::forward<T>(u));
        }

        /**
         * @brief Removes a friend verification entry.
         * @param friend_user_id The ID of the friend to remove verification for.
         */
        void removeFriendVerification(long long friend_user_id)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_friend_verification_map_mutex);
            auto itor = m_user_friend_verification_map.find(friend_user_id);
            if (itor == m_user_friend_verification_map.end())
                throw std::invalid_argument("There is not any friend verification!");
            m_user_friend_verification_map.erase(itor);
        }

        /**
         * @brief Retrieves the list of friend verification entries.
         * @return unordered_map containing friend verification entries.
         */
        std::unordered_map<long long,
            UserVerificationStruct> getFriendVerificationList() const;

        /**
         * @brief Adds a group to the user's group list.
         * @param group_id The ID of the group to add.
         * @return true if adding group was successful, false otherwise.
         */
        bool addGroup(long long group_id);

        /**
         * @brief Adds a group verification entry.
         * @tparam T Type of UserVerificationStruct.
         * @param group_id The ID of the group.
         * @param u UserVerificationStruct to add.
         */
        template<class T, class Y =
            std::enable_if_t<std::is_same_v<
            std::remove_const_t<std::remove_reference_t<T>>,
            UserVerificationStruct>>>
        void addGroupVerification(long long group_id, T&& u)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_group_verification_map_mutex);
            m_user_group_verification_map.insert({ group_id, std::forward<T>(u) });
        }

        /**
         * @brief Removes a group verification entry.
         * @param group_id The ID of the group to remove verification for.
         * @param user_id The ID of the user to remove verification for.
         */
        void removeGroupVerification(long long group_id, long long user_id)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_group_verification_map_mutex);
            size_t size = m_user_group_verification_map.count(group_id);
            if (!size) throw std::invalid_argument("There is not any group verification!");

            auto itor = m_user_group_verification_map.find(group_id);
            for (; itor->first == group_id && itor != m_user_group_verification_map.end(); itor++)
            {
                if (itor->second.user_id == user_id)
                {
                    m_user_group_verification_map.erase(itor);
                    break;
                }
            }
        }

        /**
         * @brief Retrieves the list of group verification entries.
         * @return multimap containing group verification entries.
         */
        std::multimap<long long,
            UserVerificationStruct> getGroupVerificationList() const;

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
        long long                   user_id; ///< User ID
        std::string                 user_name; ///< User name
        long long                   registered_time; ///< Time when user registered
        int                         age; ///< User's age
        std::string                 email; ///< User's email
        std::string                 phone; ///< User's phone number
        std::string                 profile; ///< User profile

        std::string                 password; ///< User's hashed password
        std::string                 salt; ///< Salt used in password hashing

        mutable std::shared_mutex   m_data_mutex; ///< Mutex for thread-safe access to user data

        std::unordered_set<long long>   m_user_friend_map; ///< User's friend list
        mutable std::shared_mutex       m_user_friend_map_mutex; ///< Mutex for thread-safe access to friend list

        std::unordered_map<long long,
            UserVerificationStruct>     m_user_friend_verification_map; ///< User's friend verification map
        mutable std::shared_mutex       m_user_friend_verification_map_mutex; ///< Mutex for thread-safe access to friend verification map

        std::unordered_set<long long>   m_user_group_map; ///< User's group list
        mutable std::shared_mutex       m_user_group_map_mutex; ///< Mutex for thread-safe access to group list

        std::multimap<long long,
            UserVerificationStruct>     m_user_group_verification_map; ///< User's group verification map
        mutable std::shared_mutex       m_user_group_verification_map_mutex; ///< Mutex for thread-safe access to group verification map

        std::unordered_map<std::shared_ptr<qls::Socket>, DeviceType>
                                        m_socket_map; ///< Map of sockets associated with the user
        mutable std::shared_mutex       m_socket_map_mutex; ///< Mutex for thread-safe access to socket map
    };
}

#endif // !USER_H
