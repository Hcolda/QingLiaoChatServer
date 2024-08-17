#include "manager.h"

#include <system_error>
#include <Ini.h>

#include "user.h"
#include "dataPackage.h"
#include "qls_error.h"

extern qini::INIObject serverIni;

namespace qls
{

void Manager::init()
{
    // sql 初始化
    // m_sqlProcess.setSQLServerInfo(serverIni["mysql"]["username"],
    //     serverIni["mysql"]["password"],
    //     "mysql",
    //     serverIni["mysql"]["host"],
    //     unsigned short(std::stoi(serverIni["mysql"]["port"])));

    // m_sqlProcess.connectSQLServer();

    {
        m_newUserId = 10000;
        m_newPrivateRoomId = 10000;
        m_newGroupRoomId = 10000;

        // sql更新初始化数据
        // ...
    }

    m_dataManager.init();
    m_verificationManager.init();
}

long long Manager::addPrivateRoom(long long user1_id, long long user2_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock1(m_basePrivateRoom_map_mutex, std::defer_lock),
        local_unique_lock2(m_userID_to_privateRoomID_map_mutex, std::defer_lock);
    std::lock(local_unique_lock1, local_unique_lock2);

    // 私聊房间id
    long long privateRoom_id = m_newGroupRoomId++;
    {
        /*
        * 这里有申请sql 创建私聊房间等命令
        */
    }

    m_basePrivateRoom_map[privateRoom_id] = std::make_shared<qls::PrivateRoom>(
        user1_id, user2_id, true);
    m_userID_to_privateRoomID_map[{user1_id, user2_id}] = privateRoom_id;

    return privateRoom_id;
}

long long Manager::getPrivateRoomId(long long user1_id, long long user2_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_userID_to_privateRoomID_map_mutex);
    if (m_userID_to_privateRoomID_map.find(
        { user1_id , user2_id }) != m_userID_to_privateRoomID_map.end())
    {
        return m_userID_to_privateRoomID_map.find({ user1_id , user2_id })->second;
    }
    else if (m_userID_to_privateRoomID_map.find(
        { user2_id , user1_id }) != m_userID_to_privateRoomID_map.end())
    {
        return m_userID_to_privateRoomID_map.find({ user2_id , user1_id })->second;
    }
    else throw std::system_error(qls_errc::private_room_not_existed);
}

bool Manager::hasPrivateRoom(long long private_room_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_basePrivateRoom_map_mutex);
    return m_basePrivateRoom_map.find(
        private_room_id) != m_basePrivateRoom_map.end();
}

std::shared_ptr<qls::PrivateRoom> Manager::getPrivateRoom(long long private_room_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_basePrivateRoom_map_mutex);
    auto itor = m_basePrivateRoom_map.find(private_room_id);
    if (itor == m_basePrivateRoom_map.end())
        throw std::system_error(qls_errc::private_room_not_existed);
    return itor->second;
}

void Manager::removePrivateRoom(long long private_room_id)
{
    std::unique_lock<std::shared_mutex> local_unique_lock1(m_basePrivateRoom_map_mutex, std::defer_lock),
        local_unique_lock2(m_userID_to_privateRoomID_map_mutex, std::defer_lock);
    std::lock(local_unique_lock1, local_unique_lock2);

    auto itor = m_basePrivateRoom_map.find(private_room_id);
    if (itor == m_basePrivateRoom_map.end())
        throw std::system_error(qls_errc::private_room_not_existed);

    {
        /*
        * 这里有申请sql 删除私聊房间等命令
        */
    }

    long long user1_id = itor->second->getUserID1();
    long long user2_id = itor->second->getUserID2();

    if (m_userID_to_privateRoomID_map.find(
        { user1_id , user2_id }) != m_userID_to_privateRoomID_map.end())
    {
        m_userID_to_privateRoomID_map.erase({ user1_id , user2_id });
    }
    else if (m_userID_to_privateRoomID_map.find(
        { user2_id , user1_id }) != m_userID_to_privateRoomID_map.end())
    {
        m_userID_to_privateRoomID_map.erase({ user2_id , user1_id });
    }

    m_basePrivateRoom_map.erase(itor);
}

long long Manager::addGroupRoom(long long opreator_user_id)
{
    std::unique_lock<std::shared_mutex> lock(m_baseRoom_map_mutex);

    // 新群聊id
    long long group_room_id = m_newPrivateRoomId++;
    {
        /*
        * sql 创建群聊获取群聊id
        */
    }

    m_baseRoom_map[group_room_id] = std::make_shared<qls::GroupRoom>(
        group_room_id, opreator_user_id, true);

    return group_room_id;
}

bool Manager::hasGroupRoom(long long group_room_id) const
{
    std::shared_lock<std::shared_mutex> lock(m_baseRoom_map_mutex);
    return m_baseRoom_map.find(group_room_id) !=
        m_baseRoom_map.end();
}

std::shared_ptr<qls::GroupRoom> Manager::getGroupRoom(long long group_room_id) const
{
    std::shared_lock<std::shared_mutex> lock(m_baseRoom_map_mutex);
    auto itor = m_baseRoom_map.find(group_room_id);
    if (itor == m_baseRoom_map.end())
        throw std::system_error(qls_errc::group_room_not_existed);
    return itor->second;
}

void Manager::removeGroupRoom(long long group_room_id)
{
    std::unique_lock<std::shared_mutex> lock(m_baseRoom_map_mutex);
    auto itor = m_baseRoom_map.find(group_room_id);
    if (itor == m_baseRoom_map.end())
        throw std::system_error(qls_errc::group_room_not_existed);

    {
        /*
        * sql删除群聊
        */
    }

    m_baseRoom_map.erase(group_room_id);
}

std::shared_ptr<qls::User> Manager::addNewUser()
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_user_map_mutex);

    long long newUserId = m_newUserId++;
    {
        // sql处理数据
    }

    m_user_map[newUserId] = std::make_shared<qls::User>(newUserId, true);

    return m_user_map[newUserId];
}

bool Manager::hasUser(long long user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_map_mutex);
    return m_user_map.find(user_id) != m_user_map.end();
}

std::shared_ptr<qls::User> Manager::getUser(long long user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_map_mutex);

    auto itor = m_user_map.find(user_id);
    if (itor == m_user_map.end())
        throw std::system_error(qls_errc::user_not_existed);
    
    return itor->second;
}

std::unordered_map<long long, std::shared_ptr<qls::User>> Manager::getUserList() const
{
    std::shared_lock lock(m_user_map_mutex);
    return m_user_map;
}

void Manager::registerSocket(const std::shared_ptr<Socket> &socket_ptr)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_socket_map_mutex);
    if (m_socket_map.find(socket_ptr) != m_socket_map.cend())
        throw std::system_error(qls_errc::socket_pointer_existed);
    m_socket_map.emplace(socket_ptr, -1ll);
}

bool Manager::hasSocket(const std::shared_ptr<Socket> &socket_ptr) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_socket_map_mutex);
    return m_socket_map.find(socket_ptr) != m_socket_map.cend();
}

bool Manager::matchUserOfSocket(const std::shared_ptr<Socket> &socket_ptr, long long user_id) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_socket_map_mutex);
    auto iter = m_socket_map.find(socket_ptr);
    if (iter == m_socket_map.cend()) return false;
    return iter->second == user_id;
}

long long Manager::getUserIDOfSocket(const std::shared_ptr<Socket> &socket_ptr) const
{
    std::shared_lock<std::shared_mutex> local_shared_lock(m_socket_map_mutex);
    auto iter = m_socket_map.find(socket_ptr);
    if (iter == m_socket_map.cend())
        throw std::system_error(qls_errc::socket_pointer_not_existed);
    return iter->second;
}

void Manager::modifyUserOfSocket(const std::shared_ptr<Socket> &socket_ptr, long long user_id, DeviceType type)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_socket_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_map_mutex, std::defer_lock);
    std::lock(local_unique_lock, local_shared_lock);

    if (m_user_map.find(user_id) == m_user_map.cend())
        throw std::system_error(qls_errc::user_not_existed);

    auto iter = m_socket_map.find(socket_ptr);
    if (iter == m_socket_map.cend())
        throw std::system_error(qls_errc::socket_pointer_not_existed);

    if (iter->second != -1ll)
        m_user_map.find(iter->second)->second->removeSocket(socket_ptr);
    m_user_map.find(user_id)->second->addSocket(socket_ptr, type);
    iter->second = user_id;
}

void qls::Manager::removeSocket(const std::shared_ptr<Socket> &socket_ptr)
{
    std::unique_lock<std::shared_mutex> local_unique_lock(m_socket_map_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> local_shared_lock(m_user_map_mutex, std::defer_lock);
    std::lock(local_unique_lock, local_shared_lock);

    auto iter = m_socket_map.find(socket_ptr);
    if (iter == m_socket_map.cend())
        throw std::system_error(qls_errc::socket_pointer_not_existed);

    if (iter->second != -1l)
        m_user_map.find(iter->second)->second->removeSocket(socket_ptr);
    
    m_socket_map.erase(iter);
}

qls::SQLDBProcess &Manager::getServerSqlProcess()
{
    return m_sqlProcess;
}

qls::DataManager &Manager::getServerDataManager()
{
    return m_dataManager;
}

qls::VerificationManager &Manager::getServerVerificationManager()
{
    return m_verificationManager;
}

} // namespace qls
