#include "user.h"

#include <chrono>

#include "manager.h"

// 服务器manager
extern qls::Manager serverManager;

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

    std::unordered_set<long long> User::getFriendList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_friend_map_mutex);
        return m_user_friend_map;
    }

    std::unordered_set<long long> User::getGroupList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_group_map_mutex);
        return m_user_group_map;
    }

    void User::updateFriendList(std::unordered_set<long long> set)
    {
        std::unique_lock<std::shared_mutex> ul(m_user_friend_map_mutex);
        this->m_user_friend_map = std::move(set);
    }

    void User::updateGroupList(std::unordered_set<long long> set)
    {
        std::unique_lock<std::shared_mutex> ul(m_user_group_map_mutex);
        m_user_group_map = std::move(set);
    }
    
    bool User::addFriend(long long friend_user_id)
    {
        if (!serverManager.hasFriendRoomVerification(this->user_id,
            friend_user_id))
        {
            serverManager.addFriendRoomVerification(this->user_id,
                friend_user_id);
            serverManager.setFriendVerified(this->user_id, friend_user_id,
                this->user_id, true);
            return true;
        }
        else return false;
    }

    bool User::addGroup(long long group_user_id)
    {
        if (!serverManager.hasGroupRoomVerification(group_user_id,
            this->user_id))
        {
            serverManager.addGroupRoomVerification(group_user_id,
                this->user_id);
            serverManager.setGroupRoomUserVerified(group_user_id,
                this->user_id, true);
            return true;
        }
        else return false;
    }
}

