#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <shared_mutex>

namespace qls
{
    class User
    {
    public:
        User(long long user_id);
        ~User() = default;

        void init();

        // 以下是获取用户的信息
        
        std::string getUserName() const;
        long long   getRegisteredTime() const;
        int         getAge() const;
        std::string getUserEmail() const;
        std::string getUserPhone() const;
        std::string getUserProfile() const;

        // 以下是修改用户信息

        void updateUserName(const std::string&);
        void updateAge(int);
        void updateUserEmail(const std::string&);
        void updateUserPhone(const std::string&);
        void updateUserProfile(const std::string&);

        // 获取用户关联信息

        bool userHasFriend(long long friend_user_id) const;
        bool userHasGroup(long long group_user_id) const;
        std::vector<long long> getFriendList() const;
        std::vector<long long> getGroupList() const;

    private:
        long long   user_id;
        std::string user_name;
        long long   registered_time;
        int         age;
        std::string email;
        std::string phone;
        std::string profile;

        mutable std::shared_mutex       m_data_mutex;

        std::unordered_map<long long,
            long long>                  m_user_friend_map;
        mutable std::shared_mutex       m_user_friend_map_mutex;

        std::unordered_map<long long,
            long long>                  m_user_group_map;
        mutable std::shared_mutex       m_user_group_map_mutex;
    };
}


