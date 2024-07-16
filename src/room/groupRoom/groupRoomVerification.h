#ifndef GROUP_ROOM_VERIFICATION_H
#define GROUP_ROOM_VERIFICATION_H

#include <atomic>

namespace qls
{
    /**
     * @brief Class representing verification status for a user within a group chat room.
     */
    class GroupRoomVerification
    {
    public:
        /**
         * @brief Constructor to initialize GroupRoomVerification object.
         * @param group_id ID of the group.
         * @param user_id ID of the user.
         */
        GroupRoomVerification(long long group_id, long long user_id);

        /**
         * @brief Copy constructor.
         * @param grv Another GroupRoomVerification object to copy from.
         */
        GroupRoomVerification(const GroupRoomVerification&);

        /**
         * @brief Move constructor.
         * @param grv Another GroupRoomVerification object to move from.
         */
        GroupRoomVerification(GroupRoomVerification&&) noexcept;

        /**
         * @brief Destructor (defaulted).
         */
        ~GroupRoomVerification() = default;

        /**
         * @brief Sets the verification status of the group chat room.
         */
        void setGroupVerified();

        /**
         * @brief Gets the verification status of the group chat room.
         * @return true if group is verified, false otherwise.
         */
        bool getGroupVerified() const;

        /**
         * @brief Sets the verification status of the user.
         */
        void setUserVerified();

        /**
         * @brief Gets the verification status of the user.
         * @return true if user is verified, false otherwise.
         */
        bool getUserVerified() const;

    private:
        const long long m_group_id; ///< ID of the group
        const long long m_user_id;  ///< ID of the user
        std::atomic<bool> m_group_is_verified; ///< Atomic flag for group verification status
        std::atomic<bool> m_user_is_verified;  ///< Atomic flag for user verification status
    };
}

#endif // !GROUP_ROOM_VERIFICATION_H
