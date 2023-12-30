#include "UserManager.h"

#include <chrono>
#include <shared_mutex>
#include <unordered_map>

namespace qls
{
    struct User::UserImpl
    {
        long long   user_id;
        std::string user_name;
        long long   registered_time;
        int         age;
        std::string email;
        std::string phone;
        std::string profile;

        std::shared_mutex data_mutex;

        std::unordered_map<long long,
            long long>                  user_friend_map;
        std::shared_mutex               user_friend_map_mutex;

        std::unordered_map<long long,
            long long>                  user_group_map;
        std::shared_mutex               user_group_map_mutex;
    };

    User::User(long long user_id)
    {
        m_UserImpl = std::make_unique<UserImpl>();

        m_UserImpl->user_id = user_id;

        {
            /*
            * sql
            */
        }
    }

    void User::init()
    {
    }

    std::string User::getUserName() const
    {
        std::shared_lock<std::shared_mutex> lock(m_UserImpl->data_mutex);
        return m_UserImpl->user_name;
    }

    long long User::getRegisteredTime() const
    {
        std::shared_lock<std::shared_mutex> lock(m_UserImpl->data_mutex);
        return m_UserImpl->registered_time;
    }

    int User::getAge() const
    {
        std::shared_lock<std::shared_mutex> lock(m_UserImpl->data_mutex);
        return m_UserImpl->age;
    }

    std::string User::getUserEmail() const
    {
        std::shared_lock<std::shared_mutex> lock(m_UserImpl->data_mutex);
       return m_UserImpl->email;
    }
    std::string User::getUserPhone() const
    {
        std::shared_lock<std::shared_mutex> lock(m_UserImpl->data_mutex);
        return m_UserImpl->phone;
    }

    std::string User::getUserProfile() const
    {
        std::shared_lock<std::shared_mutex> lock(m_UserImpl->data_mutex);
        return m_UserImpl->profile;
    }

    void User::updateUserName(const std::string& user_name)
    {
        std::lock_guard<std::shared_mutex> lg(m_UserImpl->data_mutex);
        m_UserImpl->user_name = user_name;
    }

    void User::updateAge(int age)
    {
        std::lock_guard<std::shared_mutex> lg(m_UserImpl->data_mutex);
        m_UserImpl->age = age;
    }

    void User::updateUserEmail(const std::string& email)
    {
        std::lock_guard<std::shared_mutex> lg(m_UserImpl->data_mutex);
        m_UserImpl->email = email;
    }

    void User::updateUserPhone(const std::string& phone)
    {
        std::lock_guard<std::shared_mutex> lg(m_UserImpl->data_mutex);
        m_UserImpl->phone = phone;
    }

    void User::updateUserProfile(const std::string& profile)
    {
        std::lock_guard<std::shared_mutex> lg(m_UserImpl->data_mutex);
        m_UserImpl->profile = profile;
    }

    bool User::userHasFriend(long long friend_user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_UserImpl->user_friend_map_mutex);
        return m_UserImpl->user_friend_map.find(friend_user_id) !=
            m_UserImpl->user_friend_map.cend();
    }

    bool User::userHasGroup(long long group_user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(m_UserImpl->user_group_map_mutex);
        return m_UserImpl->user_group_map.find(group_user_id) !=
            m_UserImpl->user_group_map.cend();
    }
}

