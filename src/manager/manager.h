#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "SQLProcess.hpp"
#include "definition.hpp"
#include "privateRoom.h"
#include "groupRoom.h"
#include "user.h"
#include "socket.h"
#include "structHasher.h"
#include "verificationManager.h"
#include "dataManager.h"

namespace qls
{
    /**
     * @class Manager
     * @brief Manages the core functionalities such as user, private room, and group room management.
     */
    class Manager
    {
    public:
        Manager() = default;
        Manager(const Manager&) = delete;
        Manager(Manager&) = delete;
        ~Manager();

        /**
         * @brief Initializes the manager.
         */
        void init();

        /**
         * @brief Adds a user socket to the global room.
         * @param user_id User ID.
         * @param socket_ptr Pointer to the user socket.
         */
        void addUserSocket2GlobalRoom(long long user_id, const std::shared_ptr<Socket>& socket_ptr);

        /**
         * @brief Retrieves the global room.
         * @return Shared pointer to the global room.
         */
        std::shared_ptr<BaseRoom> getGlobalRoom() const;

        /**
         * @brief Adds a private room between two users.
         * @param user1_id ID of the first user.
         * @param user2_id ID of the second user.
         * @return The ID of the created private room.
         */
        long long addPrivateRoom(long long user1_id, long long user2_id);

        /**
         * @brief Retrieves the private room ID between two users.
         * @param user1_id ID of the first user.
         * @param user2_id ID of the second user.
         * @return The ID of the private room.
         */
        long long getPrivateRoomId(long long user1_id, long long user2_id) const;

        /**
         * @brief Checks if a private room exists.
         * @param private_room_id The ID of the private room.
         * @return True if the private room exists, false otherwise.
         */
        bool hasPrivateRoom(long long private_room_id) const;

        /**
         * @brief Retrieves a private room.
         * @param private_room_id The ID of the private room.
         * @return Shared pointer to the private room.
         */
        std::shared_ptr<qls::PrivateRoom> getPrivateRoom(long long private_room_id) const;

        /**
         * @brief Removes a private room.
         * @param private_room_id The ID of the private room.
         */
        void removePrivateRoom(long long private_room_id);

        /**
         * @brief Adds a group room.
         * @param operator_user_id ID of the user creating the group room.
         * @return The ID of the created group room.
         */
        long long addGroupRoom(long long operator_user_id);

        /**
         * @brief Checks if a group room exists.
         * @param group_room_id The ID of the group room.
         * @return True if the group room exists, false otherwise.
         */
        bool hasGroupRoom(long long group_room_id) const;

        /**
         * @brief Retrieves a group room.
         * @param group_room_id The ID of the group room.
         * @return Shared pointer to the group room.
         */
        std::shared_ptr<qls::GroupRoom> getGroupRoom(long long group_room_id) const;

        /**
         * @brief Removes a group room.
         * @param group_room_id The ID of the group room.
         */
        void removeGroupRoom(long long group_room_id);

        /**
         * @brief Adds a new user.
         * @return Shared pointer to the new user.
         */
        std::shared_ptr<qls::User> addNewUser();

        /**
         * @brief Checks if a user exists.
         * @param user_id The ID of the user.
         * @return True if the user exists, false otherwise.
         */
        bool hasUser(long long user_id) const;

        /**
         * @brief Retrieves a user.
         * @param user_id The ID of the user.
         * @return Shared pointer to the user.
         */
        std::shared_ptr<qls::User> getUser(long long user_id) const;

        /**
         * @brief Retrieves the list of users.
         * @return Unordered map of user IDs to user shared pointers.
         */
        std::unordered_map<long long, std::shared_ptr<qls::User>> getUserList() const;

        /**
         * @brief Retrieves the SQL process for the server.
         * @return Reference to the SQLDBProcess.
         */
        quqisql::SQLDBProcess& getServerSqlProcess();

        /**
         * @brief Retrieves the data manager for the server.
         * @return Reference to the DataManager.
         */
        qls::DataManager& getServerDataManager();

        /**
         * @brief Retrieves the verification manager for the server.
         * @return Reference to the VerificationManager.
         */
        qls::VerificationManager& getServerVerificationManager();

    private:
        qls::DataManager                        m_dataManager; ///< Data manager instance.
        qls::VerificationManager                m_verificationManager; ///< Verification manager instance.

        std::shared_ptr<BaseRoom>               m_globalRoom; ///< Global room instance.

        // Group room map
        std::unordered_map<long long, std::shared_ptr<qls::GroupRoom>> m_baseRoom_map; ///< Map of group room IDs to group rooms.
        mutable std::shared_mutex               m_baseRoom_map_mutex; ///< Mutex for group room map.

        // Private room map
        std::unordered_map<long long, std::shared_ptr<qls::PrivateRoom>> m_basePrivateRoom_map; ///< Map of private room IDs to private rooms.
        mutable std::shared_mutex               m_basePrivateRoom_map_mutex; ///< Mutex for private room map.

        // Map of user IDs to private room IDs
        std::unordered_map<PrivateRoomIDStruct, long long, PrivateRoomIDStructHasher> m_userID_to_privateRoomID_map; ///< Map of user ID structs to private room IDs.
        mutable std::shared_mutex               m_userID_to_privateRoomID_map_mutex; ///< Mutex for user to private room ID map.

        // User map
        std::unordered_map<long long, std::shared_ptr<qls::User>> m_user_map; ///< Map of user IDs to users.
        mutable std::shared_mutex               m_user_map_mutex; ///< Mutex for user map.

        // New user ID
        std::atomic<long long>                  m_newUserId; ///< Atomic counter for new user IDs.
        // New private room ID
        std::atomic<long long>                  m_newPrivateRoomId; ///< Atomic counter for new private room IDs.
        // New group room ID
        std::atomic<long long>                  m_newGroupRoomId; ///< Atomic counter for new group room IDs.

        // SQL process manager
        quqisql::SQLDBProcess                   m_sqlProcess; ///< SQL process instance.
    };
}

#endif // !MANAGER_H
