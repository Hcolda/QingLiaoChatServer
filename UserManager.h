#pragma once

#include <string>
#include <memory>

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

    private:
        struct UserImpl;

        std::unique_ptr<UserImpl> m_UserImpl;
    };
}


