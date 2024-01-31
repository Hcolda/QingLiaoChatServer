#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <shared_mutex>
#include <string>

namespace qls
{
    class User
    {
    public:
        struct UserVerificationStruct
        {
            enum class VerificationType
            {
                Unknown = 0,
                Sent,
                Received
            };

            long long user_id = 0;
            /*
            * @brief 验证类型:
            * @brief {Unknown:  无状态}
            * @brief {Sent:     发出的申请}
            * @brief {Received: 接收的申请}
            */
            VerificationType verification_type =
                VerificationType::Unknown;
            bool has_message = false;
            std::string message;
        };

        User(long long user_id);
        ~User() = default;

        void init();

        // 以下是获取用户的信息
        
        long long   getUserID() const;
        std::string getUserName() const;
        long long   getRegisteredTime() const;
        int         getAge() const;
        std::string getUserEmail() const;
        std::string getUserPhone() const;
        std::string getUserProfile() const;
        bool        isUserPassword(const std::string&) const;

        // 以下是修改用户信息

        void updateUserName(const std::string&);
        void updateAge(int);
        void updateUserEmail(const std::string&);
        void updateUserPhone(const std::string&);
        void updateUserProfile(const std::string&);
        void firstUpdateUserPassword(const std::string& new_password);
        void updateUserPassword(const std::string& old_password,
            const std::string& new_password);

        // 获取用户关联信息
        // 采用复制的方式，保证内存安全！！！

        bool userHasFriend(long long friend_user_id) const;
        bool userHasGroup(long long group_user_id) const;
        std::unordered_set<long long> getFriendList() const;
        std::unordered_set<long long> getGroupList() const;
        void updateFriendList(std::unordered_set<long long>);
        void updateGroupList(std::unordered_set<long long>);

        /*
        * @brief 添加好友申请
        * @param friend_user_id 好友id
        * @return true 提交申请成功 | false 失败
        */
        bool addFriend(long long friend_user_id);

        template<class T, class Y =
            std::enable_if_t<std::is_same_v<
            std::remove_const_t<std::remove_reference_t<T>>,
            UserVerificationStruct>>>
        void addFriendVerification(long long friend_user_id, T&& u)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_friend_verification_map_mutex);
            m_user_friend_verification_map.emplace(friend_user_id, std::forward<T>(u));
        }

        void removeFriendVerification(long long friend_user_id)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_friend_verification_map_mutex);
            auto itor = m_user_friend_verification_map.find(friend_user_id);
            if (itor == m_user_friend_verification_map.end())
                throw std::invalid_argument("Wrong argument!");
            m_user_friend_verification_map.erase(itor);
        }

        /*
        * @brief 获取用户申请表
        * @return unordered_map
        */
        std::unordered_map<long long,
            UserVerificationStruct> getFriendVerificationList() const;

        /*
        * @brief 添加群申请
        * @param group_user_id 群聊id
        * @return true 提交申请成功 | false 失败
        */
        bool addGroup(long long group_user_id);

        template<class T, class Y =
            std::enable_if_t<std::is_same_v<
            std::remove_const_t<std::remove_reference_t<T>>,
            UserVerificationStruct>>>
        void addGroupVerification(long long group_id, T&& u)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_group_verification_map_mutex);
            m_user_group_verification_map.insert({ group_id, std::forward<T>(u) });
        }

        void removeGroupVerification(long long group_id, long long user_id)
        {
            std::unique_lock<std::shared_mutex> ul(m_user_group_verification_map_mutex);
            size_t size = m_user_group_verification_map.count(group_id);
            if (!size) throw std::invalid_argument("Wrong argument!");

            auto itor = m_user_group_verification_map.find(group_id);
            for (; itor->first == group_id && itor != m_user_group_verification_map.end(); itor++)
            {
                if (itor->second.user_id == user_id)
                {
                    m_user_group_verification_map.erase(itor);
                    break;
                }
            }
        }

        /*
        * @brief 获取群聊申请验证
        */
        std::multimap<long long,
            UserVerificationStruct> getGroupVerificationList() const;

    private:
        // 用户数据
        long long                   user_id;
        std::string                 user_name;
        long long                   registered_time;
        int                         age;
        std::string                 email;
        std::string                 phone;
        std::string                 profile;

        // 用户密码（hash）
        std::string                 password;
        std::string                 salt;

        mutable std::shared_mutex   m_data_mutex;

        // 用户friendlist
        std::unordered_set<long long>   m_user_friend_map;
        mutable std::shared_mutex       m_user_friend_map_mutex;

        // 用户friendVerification
        std::unordered_map<long long,
            UserVerificationStruct>     m_user_friend_verification_map;
        mutable std::shared_mutex       m_user_friend_verification_map_mutex;

        // 用户grouplist
        std::unordered_set<long long>   m_user_group_map;
        mutable std::shared_mutex       m_user_group_map_mutex;

        // 用户的groupVerification
        std::multimap<long long,
            UserVerificationStruct>     m_user_group_verification_map;
        mutable std::shared_mutex       m_user_group_verification_map_mutex;
    };
}
