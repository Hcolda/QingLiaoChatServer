#include "user.h"

#include <chrono>
#include <random>
#include <asio.hpp>

#include "manager.h"
#include "Logger.hpp"
#include "qls_error.h"

// 服务器log系统
extern Log::Logger serverLogger;
// 服务器manager
extern qls::Manager serverManager;

namespace qls
{
    User::User(long long user_id, bool is_create) :
        user_id(user_id),
        age(0),
        registered_time(std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count())
    {
        if (is_create)
        {
            // sql 创建用户
        }
        else
        {
            // sql 读取用户信息
        }
    }

    long long User::getUserID() const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_data_mutex);
        return this->user_id;
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

    bool User::isUserPassword(const std::string& p) const
    {
        std::hash<std::string> string_hash;
        std::shared_lock<std::shared_mutex> sl(m_data_mutex);
        std::string localPassword = p + salt;

        for (size_t i = 0; i < 256ull; i++)
        {
            localPassword = std::to_string(string_hash(localPassword));
        }

        return localPassword == password;
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

    void User::firstUpdateUserPassword(const std::string& new_password)
    {
        if (!password.empty())
            throw std::system_error(qls_errc::password_already_set);

        std::hash<std::string>  string_hash;
        std::mt19937_64         mt(std::random_device{}());

        size_t mt_temp = 0;
        for (size_t i = 0; i < 256ull; i++)
        {
            mt_temp = mt();
        }

        std::string localSalt = std::to_string(mt_temp);
        std::string localPassword = new_password + localSalt;

        for (size_t i = 0; i < 256ull; i++)
        {
            localPassword = std::to_string(string_hash(localPassword));
        }

        {
            std::unique_lock<std::shared_mutex> ul(m_data_mutex);
            password = localPassword;
            salt = localSalt;
        }
        {
            // sql
        }
    }

    void User::updateUserPassword(const std::string& old_password, const std::string& new_password)
    {
        if (!this->isUserPassword(old_password))
            throw std::system_error(qls_errc::password_mismatched, "wrong old password");

        std::hash<std::string>  string_hash;
        std::mt19937_64         mt(std::random_device{}());

        size_t mt_temp = 0;
        for (size_t i = 0; i < 256ull; i++)
        {
            mt_temp = mt();
        }

        std::string localSalt = std::to_string(mt_temp);
        std::string localPassword = new_password + localSalt;

        for (size_t i = 0; i < 256ull; i++)
        {
            localPassword = std::to_string(string_hash(localPassword));
        }

        {
            std::unique_lock<std::shared_mutex> ul(m_data_mutex);
            password = localPassword;
            salt = localSalt;
        }
        {
            //sql
        }
    }

    bool User::userHasFriend(long long friend_user_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_user_friend_map_mutex);
        return this->m_user_friend_map.find(friend_user_id) !=
            this->m_user_friend_map.cend();
    }

    bool User::userHasGroup(long long group_id) const
    {
        std::shared_lock<std::shared_mutex> sl(this->m_user_group_map_mutex);
        return this->m_user_group_map.find(group_id) !=
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
        auto& ver = serverManager.getServerVerificationManager();
        if (!ver.hasFriendRoomVerification(this->user_id,
            friend_user_id))
        {
            ver.addFriendRoomVerification(this->user_id,
                friend_user_id);
            ver.setFriendVerified(this->user_id, friend_user_id,
                this->user_id);
            return true;
        }
        else return false;
    }

    std::unordered_map<long long, UserVerificationStruct> User::getFriendVerificationList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_friend_verification_map_mutex);
        return m_user_friend_verification_map;
    }

    bool User::addGroup(long long group_id)
    {
        auto& ver = serverManager.getServerVerificationManager();
        if (!ver.hasGroupRoomVerification(group_id,
            this->user_id))
        {
            ver.addGroupRoomVerification(group_id,
                this->user_id);
            ver.setGroupRoomUserVerified(group_id,
                this->user_id);
            return true;
        }
        else return false;
    }

    std::multimap<long long, UserVerificationStruct> User::getGroupVerificationList() const
    {
        std::shared_lock<std::shared_mutex> sl(m_user_group_verification_map_mutex);
        return m_user_group_verification_map;
    }

    void User::addSocket(const std::shared_ptr<qls::Socket> &socket_ptr, DeviceType type)
    {
        std::unique_lock<std::shared_mutex> ul(m_socket_map_mutex);
        if (m_socket_map.find(socket_ptr) == m_socket_map.cend())
            throw std::system_error(qls_errc::socket_pointer_existed);

        m_socket_map.emplace(socket_ptr, type);
    }

    bool User::hasSocket(const std::shared_ptr<qls::Socket> &socket_ptr) const
    {
        std::shared_lock<std::shared_mutex> sl(m_socket_map_mutex);
        return m_socket_map.find(socket_ptr) != m_socket_map.cend();
    }

    void User::modifySocketType(const std::shared_ptr<qls::Socket> &socket_ptr, DeviceType type)
    {
        std::unique_lock<std::shared_mutex> ul(m_socket_map_mutex);
        auto iter = m_socket_map.find(socket_ptr);
        if (iter == m_socket_map.cend())
            throw std::system_error(qls_errc::null_socket_pointer, "socket pointer doesn't exist");

        iter->second = type;
    }

    void User::removeSocket(const std::shared_ptr<qls::Socket>& socket_ptr)
    {
        std::unique_lock<std::shared_mutex> ul(m_socket_map_mutex);
        auto iter = m_socket_map.find(socket_ptr);
        if (iter == m_socket_map.cend())
            throw std::system_error(qls_errc::null_socket_pointer, "socket pointer doesn't exist");
        
        m_socket_map.erase(iter);
    }

    void User::notifyAll(std::string_view data)
    {
        std::shared_lock<std::shared_mutex> sl(m_socket_map_mutex);
        for (auto& [socket_ptr, type]: m_socket_map)
        {
            asio::async_write(*socket_ptr, asio::buffer(data),
                [this](std::error_code ec, size_t n){
                    if (ec) serverLogger.error('[', ec.category().name(), ']',
                        ec.message());
                });
        }
    }
    
    void User::notifyWithType(DeviceType type, std::string_view data)
    {
        std::shared_lock<std::shared_mutex> sl(m_socket_map_mutex);
        for (auto& [socket_ptr, dtype]: m_socket_map)
        {
            if (dtype == type)
            {
                asio::async_write(*socket_ptr, asio::buffer(data),
                    [this](std::error_code ec, size_t n){
                        if (ec) serverLogger.error('[', ec.category().name(), ']',
                            ec.message());
                    });
            }
        }
    }
}
