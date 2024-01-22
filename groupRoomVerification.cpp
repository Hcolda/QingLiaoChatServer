#include "groupRoomVerification.h"

namespace qls
{
    GroupRoomVerification::GroupRoomVerification(long long group_id, long long user_id) :
        m_group_id(group_id),
        m_user_id(user_id),
        m_group_is_verified(false),
        m_user_is_verified(false)
    {
    }

    void GroupRoomVerification::setGroupVerified(bool is_verified)
    {
        m_group_is_verified = is_verified;
    }

    bool GroupRoomVerification::getGroupVerified() const
    {
        return m_group_is_verified;
    }

    void GroupRoomVerification::setUserVerified(bool is_verified)
    {
        m_user_is_verified = is_verified;
    }

    bool GroupRoomVerification::getUserVerified() const
    {
        return m_user_is_verified;
    }
}
