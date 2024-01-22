#include "user.h"

#include <chrono>

namespace qls
{

    User::User(long long user_id)
    {
        this->user_id = user_id;
        this->age = 0;
        this->registered_time = std::chrono::system_clock::now()
            .time_since_epoch().count();

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
        std::shared_lock<std::shared_mutex> lock(this->m_data_mutex);
        return this->user_name;
    }

    long long User::getRegisteredTime() const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_data_mutex);
        return this->registered_time;
    }

    int User::getAge() const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_data_mutex);
        return this->age;
    }

    std::string User::getUserEmail() const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_data_mutex);
       return this->email;
    }
    std::string User::getUserPhone() const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_data_mutex);
        return this->phone;
    }

    std::string User::getUserProfile() const
    {
        std::shared_lock<std::shared_mutex> lock(this->m_data_mutex);
        return this->profile;
    }

    void User::updateUserName(const std::string& user_name)
    {
        std::unique_lock<std::shared_mutex> lg(this->m_data_mutex);
        this->user_name = user_name;
    }

    void User::updateAge(int age)
    {
        std::unique_lock<std::shared_mutex> lg(this->m_data_mutex);
        this->age = age;
    }

    void User::updateUserEmail(const std::string& email)
    {
        std::unique_lock<std::shared_mutex> lg(this->m_data_mutex);
        this->email = email;
    }

    void User::updateUserPhone(const std::string& phone)
    {
        std::unique_lock<std::shared_mutex> lg(this->m_data_mutex);
        this->phone = phone;
    }

    void User::updateUserProfile(const std::string& profile)
    {
        std::unique_lock<std::shared_mutex> lg(this->m_data_mutex);
        this->profile = profile;
    }

    bool User::userHasFriend(long long friend_user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_user_friend_map_mutex);
        return this->m_user_friend_map.find(friend_user_id) !=
            this->m_user_friend_map.cend();
    }

    bool User::userHasGroup(long long group_user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_user_group_map_mutex);
        return this->m_user_group_map.find(group_user_id) !=
            this->m_user_group_map.cend();
    }

    std::vector<long long> User::getFriendList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_friend_map_mutex);

        std::vector<long long> localList;

        for (const auto& element : m_user_friend_map)
        {
            localList.push_back(element);
        }

        return localList;
    }

    std::vector<long long> User::getGroupList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_group_map_mutex);

        std::vector<long long> localList;

        for (const auto& element : m_user_group_map)
        {
            localList.push_back(element);
        }

        return localList;
    }
    
    bool User::addFriend(long long friend_user_id)
    {
        return false;
    }

    bool User::addGroup(long long group_user_id)
    {
        return false;
    }
}

