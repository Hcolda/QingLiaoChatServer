template<class T, class Y>
void qls::User::updateFriendList(T&& set)
{
    std::unique_lock<std::shared_mutex> ul(m_user_friend_map_mutex);
    this->m_user_friend_map = std::forward<T>(set);
}

template<class T, class Y>
void qls::User::updateGroupList(T&& set)
{
    std::unique_lock<std::shared_mutex> ul(m_user_group_map_mutex);
    m_user_group_map = std::forward<T>(set);
}

template<class T, class Y>
void qls::User::addFriendVerification(long long friend_user_id, T&& u)
{
    std::unique_lock<std::shared_mutex> ul(m_user_friend_verification_map_mutex);
    m_user_friend_verification_map.emplace(friend_user_id, std::forward<T>(u));
}

template<class T, class Y>
void qls::User::addGroupVerification(long long group_id, T&& u)
{
    std::unique_lock<std::shared_mutex> ul(m_user_group_verification_map_mutex);
    m_user_group_verification_map.insert({ group_id, std::forward<T>(u) });
}