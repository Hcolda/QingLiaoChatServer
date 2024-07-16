#include "friendRoomVerification.h"

#include <stdexcept>

qls::FriendRoomVerification::FriendRoomVerification(long long user_id_1, long long user_id_2) :
    m_user_id_1(user_id_1),
    m_user_id_2(user_id_2),
    m_user_1_is_verified(false),
    m_user_2_is_verified(false)
{
}

qls::FriendRoomVerification::FriendRoomVerification(const FriendRoomVerification& f) :
    m_user_id_1(f.m_user_id_1),
    m_user_id_2(f.m_user_id_2),
    m_user_1_is_verified(static_cast<bool>(f.m_user_1_is_verified)),
    m_user_2_is_verified(static_cast<bool>(f.m_user_2_is_verified))

{
}

qls::FriendRoomVerification::FriendRoomVerification(FriendRoomVerification&& f) noexcept :
    m_user_id_1(f.m_user_id_1),
    m_user_id_2(f.m_user_id_2),
    m_user_1_is_verified(static_cast<bool>(f.m_user_1_is_verified)),
    m_user_2_is_verified(static_cast<bool>(f.m_user_2_is_verified))
{
}

void qls::FriendRoomVerification::setUserVerified(long long user_id)
{
    if (user_id != m_user_id_1 && user_id != m_user_id_2)
        throw std::invalid_argument("Wrong user_id!");

    if (user_id == m_user_id_1)
        m_user_1_is_verified = true;
    else if (user_id == m_user_id_2)
        m_user_2_is_verified = true;
}

bool qls::FriendRoomVerification::getUserVerified(long long user_id) const
{
    if (user_id != m_user_id_1 && user_id != m_user_id_2)
        throw std::invalid_argument("Wrong user_id!");

    if (user_id == m_user_id_1)
        return m_user_1_is_verified;
    else
        return m_user_2_is_verified;
}
