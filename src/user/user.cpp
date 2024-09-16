#include "user.h"

#include <chrono>
#include <random>
#include <asio.hpp>

#include "manager.h"
#include "logger.hpp"
#include "qls_error.h"

// 服务器log系统
extern Log::Logger serverLogger;
// 服务器manager
extern qls::Manager serverManager;

namespace qls
{

struct UserImpl
{
    UserID      user_id; ///< User ID
    std::string user_name; ///< User name
    long long   registered_time; ///< Time when user registered
    int         age; ///< User's age
    std::string email; ///< User's email
    std::string phone; ///< User's phone number
    std::string     profile; ///< User profile

    std::string password; ///< User's hashed password
    std::string salt; ///< Salt used in password hashing

    std::shared_mutex   m_data_mutex; ///< Mutex for thread-safe access to user data

    std::unordered_set<UserID>      m_user_friend_map; ///< User's friend list
    std::shared_mutex               m_user_friend_map_mutex; ///< Mutex for thread-safe access to friend list

    std::unordered_map<UserID,
        UserVerificationStructure>  m_user_friend_verification_map; ///< User's friend verification map
    std::shared_mutex               m_user_friend_verification_map_mutex; ///< Mutex for thread-safe access to friend verification map

    std::unordered_set<GroupID>     m_user_group_map; ///< User's group list
    std::shared_mutex               m_user_group_map_mutex; ///< Mutex for thread-safe access to group list

    std::multimap<GroupID,
        UserVerificationStructure>  m_user_group_verification_map; ///< User's group verification map
    std::shared_mutex               m_user_group_verification_map_mutex; ///< Mutex for thread-safe access to group verification map

    std::unordered_map<std::shared_ptr<Socket>, DeviceType>
                                    m_socket_map; ///< Map of sockets associated with the user
    std::shared_mutex               m_socket_map_mutex; ///< Mutex for thread-safe access to socket map
};

User::User(UserID user_id, bool is_create):
    m_impl(std::make_unique<UserImpl>())
{
    m_impl->user_id = user_id;
    m_impl->age = 0;
    m_impl->registered_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now()).time_since_epoch().count();

    if (is_create) {
        // sql 创建用户
    }
    else {
        // sql 读取用户信息
    }
}

User::~User() = default;

UserID User::getUserID() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_data_mutex);
    return m_impl->user_id;
}

std::string User::getUserName() const
{
    std::shared_lock<std::shared_mutex> lock(m_impl->m_data_mutex);
    return m_impl->user_name;
}

long long User::getRegisteredTime() const
{
    std::shared_lock<std::shared_mutex> lock(m_impl->m_data_mutex);
    return m_impl->registered_time;
}

int User::getAge() const
{
    std::shared_lock<std::shared_mutex> lock(m_impl->m_data_mutex);
    return m_impl->age;
}

std::string User::getUserEmail() const
{
    std::shared_lock<std::shared_mutex> lock(m_impl->m_data_mutex);
    return m_impl->email;
}
std::string User::getUserPhone() const
{
    std::shared_lock<std::shared_mutex> lock(m_impl->m_data_mutex);
    return m_impl->phone;
}

std::string User::getUserProfile() const
{
    std::shared_lock<std::shared_mutex> lock(m_impl->m_data_mutex);
    return m_impl->profile;
}

bool User::isUserPassword(const std::string& p) const
{
    std::hash<std::string> string_hash;
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_data_mutex);
    std::string localPassword = p + m_impl->salt;

    for (size_t i = 0; i < 256ull; i++) {
        localPassword = std::to_string(string_hash(localPassword));
    }

    return localPassword == m_impl->password;
}

void User::updateUserName(const std::string& user_name)
{
    std::unique_lock<std::shared_mutex> lg(m_impl->m_data_mutex);
    m_impl->user_name = user_name;
}

void User::updateAge(int age)
{
    std::unique_lock<std::shared_mutex> lg(m_impl->m_data_mutex);
    m_impl->age = age;
}

void User::updateUserEmail(const std::string& email)
{
    std::unique_lock<std::shared_mutex> lg(m_impl->m_data_mutex);
    m_impl->email = email;
}

void User::updateUserPhone(const std::string& phone)
{
    std::unique_lock<std::shared_mutex> lg(m_impl->m_data_mutex);
    m_impl->phone = phone;
}

void User::updateUserProfile(const std::string& profile)
{
    std::unique_lock<std::shared_mutex> lg(m_impl->m_data_mutex);
    m_impl->profile = profile;
}

void User::firstUpdateUserPassword(const std::string& new_password)
{
    if (!m_impl->password.empty())
        throw std::system_error(qls_errc::password_already_set);

    std::hash<std::string>  string_hash;
    std::mt19937_64         mt(std::random_device{}());

    size_t mt_temp = 0;
    for (size_t i = 0; i < 256ull; i++) {
        mt_temp = mt();
    }

    std::string localSalt = std::to_string(mt_temp);
    std::string localPassword = new_password + localSalt;

    for (size_t i = 0; i < 256ull; i++) {
        localPassword = std::to_string(string_hash(localPassword));
    }

    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_data_mutex);
        m_impl->password = localPassword;
        m_impl->salt = localSalt;
    }
    {
        // sql
    }
}

void User::updateUserPassword(const std::string& old_password, const std::string& new_password)
{
    if (!isUserPassword(old_password))
        throw std::system_error(qls_errc::password_mismatched, "wrong old password");

    std::hash<std::string>  string_hash;
    std::mt19937_64         mt(std::random_device{}());

    size_t mt_temp = 0;
    for (size_t i = 0; i < 256ull; i++) {
        mt_temp = mt();
    }

    std::string localSalt = std::to_string(mt_temp);
    std::string localPassword = new_password + localSalt;

    for (size_t i = 0; i < 256ull; i++) {
        localPassword = std::to_string(string_hash(localPassword));
    }

    {
        std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_data_mutex);
        m_impl->password = localPassword;
        m_impl->salt = localSalt;
    }
    {
        //sql
    }
}

bool User::userHasFriend(UserID friend_user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_friend_map_mutex);
    return m_impl->m_user_friend_map.find(friend_user_id) !=
        m_impl->m_user_friend_map.cend();
}

bool User::userHasGroup(GroupID group_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_group_map_mutex);
    return m_impl->m_user_group_map.find(group_id) !=
        m_impl->m_user_group_map.cend();
}

std::unordered_set<UserID> User::getFriendList() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_friend_map_mutex);
    return m_impl->m_user_friend_map;
}

std::unordered_set<GroupID> User::getGroupList() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_group_map_mutex);
    return m_impl->m_user_group_map;
}

bool User::addFriend(UserID friend_user_id)
{
    auto& ver = serverManager.getServerVerificationManager();
    if (!ver.hasFriendRoomVerification(m_impl->user_id, friend_user_id)) {
        ver.addFriendRoomVerification(m_impl->user_id, friend_user_id);
        ver.setFriendVerified(m_impl->user_id, friend_user_id, m_impl->user_id);
        return true;
    }
    else
        return false;
}

void User::updateFriendList(const std::unordered_set<UserID>& set)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_friend_map_mutex);
    m_impl->m_user_friend_map = set;
}

void User::updateFriendList(std::unordered_set<UserID>&& set)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_friend_map_mutex);
    m_impl->m_user_friend_map = std::move(set);
}

void User::updateGroupList(const std::unordered_set<GroupID>& set)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_group_map_mutex);
    m_impl->m_user_group_map = set;
}

void User::updateGroupList(std::unordered_set<GroupID>&& set)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_group_map_mutex);
    m_impl->m_user_group_map = std::move(set);
}

void User::addFriendVerification(UserID friend_user_id, const UserVerificationStructure& u)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_friend_verification_map_mutex);
    m_impl->m_user_friend_verification_map.emplace(friend_user_id, u);
}

void User::addGroupVerification(GroupID group_id, const UserVerificationStructure& u)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_group_verification_map_mutex);
    m_impl->m_user_group_verification_map.insert({ group_id, u });
}

void User::removeFriendVerification(UserID friend_user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_friend_verification_map_mutex);
    auto itor = m_impl->m_user_friend_verification_map.find(friend_user_id);
    if (itor == m_impl->m_user_friend_verification_map.cend())
        throw std::system_error(qls_errc::private_room_verification_not_existed);
    m_impl->m_user_friend_verification_map.erase(itor);
}

std::unordered_map<UserID, UserVerificationStructure> User::getFriendVerificationList() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_friend_verification_map_mutex);
    return m_impl->m_user_friend_verification_map;
}

bool User::addGroup(GroupID group_id)
{
    auto& ver = serverManager.getServerVerificationManager();
    if (!ver.hasGroupRoomVerification(group_id, m_impl->user_id)) {
        ver.addGroupRoomVerification(group_id, m_impl->user_id);
        ver.setGroupRoomUserVerified(group_id, m_impl->user_id);
        return true;
    }
    else return false;
}

void User::removeGroupVerification(GroupID group_id, UserID user_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_user_group_verification_map_mutex);
    size_t size = m_impl->m_user_group_verification_map.count(group_id);
    if (!size) throw std::system_error(qls_errc::group_room_verification_not_existed);

    auto itor = m_impl->m_user_group_verification_map.find(group_id);
    for (; itor->first == group_id && itor != m_impl->m_user_group_verification_map.cend(); itor++) {
        if (itor->second.user_id == user_id) {
            m_impl->m_user_group_verification_map.erase(itor);
            break;
        }
    }
}

std::multimap<GroupID, UserVerificationStructure> User::getGroupVerificationList() const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_user_group_verification_map_mutex);
    return m_impl->m_user_group_verification_map;
}

void User::addSocket(const std::shared_ptr<Socket> &socket_ptr, DeviceType type)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_socket_map_mutex);
    if (m_impl->m_socket_map.find(socket_ptr) == m_impl->m_socket_map.cend())
        throw std::system_error(qls_errc::socket_pointer_existed);

    m_impl->m_socket_map.emplace(socket_ptr, type);
}

bool User::hasSocket(const std::shared_ptr<Socket> &socket_ptr) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_socket_map_mutex);
    return m_impl->m_socket_map.find(socket_ptr) != m_impl->m_socket_map.cend();
}

void User::modifySocketType(const std::shared_ptr<Socket> &socket_ptr, DeviceType type)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_socket_map_mutex);
    auto iter = m_impl->m_socket_map.find(socket_ptr);
    if (iter == m_impl->m_socket_map.cend())
        throw std::system_error(qls_errc::null_socket_pointer, "socket pointer doesn't exist");

    iter->second = type;
}

void User::removeSocket(const std::shared_ptr<Socket>& socket_ptr)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_impl->m_socket_map_mutex);
    auto iter = m_impl->m_socket_map.find(socket_ptr);
    if (iter == m_impl->m_socket_map.cend())
        throw std::system_error(qls_errc::null_socket_pointer, "socket pointer doesn't exist");
    
    m_impl->m_socket_map.erase(iter);
}

void User::notifyAll(std::string_view data)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_socket_map_mutex);
    std::shared_ptr<std::string> buffer_ptr(std::make_shared<std::string>(data));
    for (auto& [socket_ptr, type]: m_impl->m_socket_map) {
        asio::async_write(*socket_ptr, asio::buffer(*buffer_ptr),
            [this, buffer_ptr](std::error_code ec, size_t n) {
                if (ec)
                    serverLogger.error('[', ec.category().name(), ']', ec.message());
            });
    }
}

void User::notifyWithType(DeviceType type, std::string_view data)
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_impl->m_socket_map_mutex);
    std::shared_ptr<std::string> buffer_ptr(std::make_shared<std::string>(data));
    for (auto& [socket_ptr, dtype]: m_impl->m_socket_map) {
        if (dtype == type) {
            asio::async_write(*socket_ptr, asio::buffer(*buffer_ptr),
                [this, buffer_ptr](std::error_code ec, size_t n) {
                    if (ec)
                        serverLogger.error('[', ec.category().name(), ']', ec.message());
                });
        }
    }
}

} // namespace qls
