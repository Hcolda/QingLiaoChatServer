#ifndef GROUP_ROOM_VERIFICATION_H
#define GROUP_ROOM_VERIFICATION_H

#include <atomic>

namespace qls
{
    class GroupRoomVerification
    {
    public:
        GroupRoomVerification(long long group_id, long long user_id);
        GroupRoomVerification(const GroupRoomVerification&);
        GroupRoomVerification(GroupRoomVerification&&) noexcept;
        ~GroupRoomVerification() = default;

        /*
        * @brief 设置群聊是否验证
        * @param is_verified 是否验证
        */
        void setGroupVerified(bool is_verified);

        /*
        * @brief 获取群聊是否验证
        * @return true 已验证 | false 未验证
        */
        bool getGroupVerified() const;

        /*
        * @brief 设置用户是否验证
        * @param is_verified 是否验证
        */
        void setUserVerified(bool is_verified);

        /*
        * @brief 获取用户是否验证
        * @return true 已验证 | false 未验证
        */
        bool getUserVerified() const;

    private:
        const long long m_group_id, m_user_id;
        std::atomic<bool> m_group_is_verified, m_user_is_verified;
    };
}

#endif // !GROUP_ROOM_VERIFICATION_H
