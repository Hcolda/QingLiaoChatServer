#ifndef VERIFICATION_MANAGER_H
#define VERIFICATION_MANAGER_H

#include <unordered_map>
#include <shared_mutex>

#include "friendRoomVerification.h"
#include "groupRoomVerification.h"
#include "structHasher.h"

namespace qls
{
    /**
     * @class VerificationManager
     * @brief Manages verifications for friend and group room requests.
     */
    class VerificationManager
    {
    public:
        VerificationManager() = default;
        ~VerificationManager() = default;

        /**
         * @brief Initializes the verification manager.
         */
        void init();

        /**
         * @brief Adds a friend room verification request between two users.
         * @param user_id_1 ID of the first user.
         * @param user_id_2 ID of the second user.
         */
        void addFriendRoomVerification(long long user_id_1, long long user_id_2);

        /**
         * @brief Checks if there is a friend room verification request between two users.
         * @param user_id_1 ID of the first user.
         * @param user_id_2 ID of the second user.
         * @return True if there is a verification request, false otherwise.
         */
        bool hasFriendRoomVerification(long long user_id_1, long long user_id_2) const;

        /**
         * @brief Sets a friend verification status.
         * @param user_id_1 ID of the first user.
         * @param user_id_2 ID of the second user.
         * @param user_id ID of the user setting the verification.
         * @return True if verification status was successfully set, false otherwise.
         */
        bool setFriendVerified(long long user_id_1, long long user_id_2, long long user_id);

        /**
         * @brief Removes a friend room verification request.
         * @param user_id_1 ID of the first user.
         * @param user_id_2 ID of the second user.
         */
        void removeFriendRoomVerification(long long user_id_1, long long user_id_2);

        /**
         * @brief Adds a group room verification request.
         * @param group_id ID of the group.
         * @param user_id ID of the user requesting to join the group.
         */
        void addGroupRoomVerification(long long group_id, long long user_id);

        /**
         * @brief Checks if there is a group room verification request.
         * @param group_id ID of the group.
         * @param user_id ID of the user requesting to join the group.
         * @return True if there is a verification request, false otherwise.
         */
        bool hasGroupRoomVerification(long long group_id, long long user_id) const;

        /**
         * @brief Sets the group verification status for a group.
         * @param group_id ID of the group.
         * @param user_id ID of the user setting the verification.
         * @return True if verification status was successfully set, false otherwise.
         */
        bool setGroupRoomGroupVerified(long long group_id, long long user_id);

        /**
         * @brief Sets the user verification status for a group.
         * @param group_id ID of the group.
         * @param user_id ID of the user setting the verification.
         * @return True if verification status was successfully set, false otherwise.
         */
        bool setGroupRoomUserVerified(long long group_id, long long user_id);

        /**
         * @brief Removes a group room verification request.
         * @param group_id ID of the group.
         * @param user_id ID of the user requesting to join the group.
         */
        void removeGroupRoomVerification(long long group_id, long long user_id);

    private:
        /**
         * @brief Map of friend room verification requests.
         */
        std::unordered_map<PrivateRoomIDStruct,
            qls::FriendRoomVerification,
            PrivateRoomIDStructHasher>          m_FriendRoomVerification_map;
        mutable std::shared_mutex               m_FriendRoomVerification_map_mutex; ///< Mutex for friend room verification map.

        /**
         * @brief Map of group room verification requests.
         */
        std::unordered_map<GroupVerificationStruct,
            qls::GroupRoomVerification,
            GroupVerificationStructHasher>      m_GroupVerification_map;
        mutable std::shared_mutex               m_GroupVerification_map_mutex; ///< Mutex for group room verification map.
    };
}

#endif // !VERIFICATION_MANAGER_H
