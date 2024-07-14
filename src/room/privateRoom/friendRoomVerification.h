#ifndef FRIEND_ROOM_VERIFICATION_H
#define FRIEND_ROOM_VERIFICATION_H

#include <atomic>

namespace qls
{
    class FriendRoomVerification
    {
    public:
        FriendRoomVerification(long long user_id_1, long long user_id_2);
        FriendRoomVerification(const FriendRoomVerification&);
        FriendRoomVerification(FriendRoomVerification&&) noexcept;
        ~FriendRoomVerification() = default;

        /*
        * @brief 设置是否验证
        * @param user_id 用户的userid
        * @param is_verified 是否验证
        */
        void setUserVerified(long long user_id);

        /*
        * @brief 获取用户是否验证
        * @param user_id 用户的userid
        * @return true 已验证 | false 未验证
        */
        bool getUserVerified(long long user_id) const;

    private:
        const long long m_user_id_1, m_user_id_2;
        std::atomic<bool> m_user_1_is_verified, m_user_2_is_verified;
    };
}

#endif // !FRIEND_ROOM_VERIFICATION_H