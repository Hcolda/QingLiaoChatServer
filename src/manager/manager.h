#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "SQLProcess.hpp"
#include "definition.hpp"
#include "privateRoom.h"
#include "groupRoom.h"
#include "user.h"
#include "socket.h"
#include "structHasher.h"
#include "verificationManager.h"
#include "dataManager.h"

namespace qls
{
    class Manager
    {
    public:
        Manager() = default;
        Manager(const Manager&) = delete;
        Manager(Manager&) = delete;
        ~Manager();

        void init();

        void addUserSocket2GlobalRoom(long long user_id, const std::shared_ptr<Socket>& socket_ptr);
        std::shared_ptr<BaseRoom> getGlobalRoom() const;

        long long addPrivateRoom(long long user1_id, long long user2_id);
        long long getPrivateRoomId(long long user1_id, long long user2_id) const;
        bool hasPrivateRoom(long long private_room_id) const;
        std::shared_ptr<qls::PrivateRoom> getPrivateRoom(long long private_room_id) const;
        void removePrivateRoom(long long private_room_id);

        long long addGroupRoom(long long opreator_user_id);
        bool hasGroupRoom(long long group_room_id) const;
        std::shared_ptr<qls::GroupRoom> getGroupRoom(long long group_room_id) const;
        void removeGroupRoom(long long group_room_id);

        std::shared_ptr<qls::User> addNewUser();
        bool hasUser(long long user_id) const;
        std::shared_ptr<qls::User> getUser(long long user_id) const;
        std::unordered_map<long long,
            std::shared_ptr<qls::User>> getUserList() const;

        quqisql::SQLDBProcess& getServerSqlProcess();
        qls::DataManager& getServerDataManager();
        qls::VerificationManager& getServerVerificationManager();

    private:
        qls::DataManager                        m_dataManager;
        qls::VerificationManager                m_verificationManager;

        std::shared_ptr<BaseRoom>               m_globalRoom;

        // 群聊房间表
        std::unordered_map<long long,
            std::shared_ptr<qls::GroupRoom>>    m_baseRoom_map;
        mutable std::shared_mutex               m_baseRoom_map_mutex;

        // 私聊房间表
        std::unordered_map<long long,
            std::shared_ptr<qls::PrivateRoom>>  m_basePrivateRoom_map;
        mutable std::shared_mutex               m_basePrivateRoom_map_mutex;

        // 私聊用户对应私聊房间号表
        std::unordered_map<PrivateRoomIDStruct,
            long long,
            PrivateRoomIDStructHasher>          m_userID_to_privateRoomID_map;
        mutable std::shared_mutex               m_userID_to_privateRoomID_map_mutex;

        // 用户表
        std::unordered_map<long long,
            std::shared_ptr<qls::User>>         m_user_map;
        mutable std::shared_mutex               m_user_map_mutex;

        // 新用户id
        std::atomic<long long>                  m_newUserId;
        // 新私聊房间id
        std::atomic<long long>                  m_newPrivateRoomId;
        // 新群聊房间id
        std::atomic<long long>                  m_newGroupRoomId;
        
        // sql进程管理
        quqisql::SQLDBProcess                   m_sqlProcess;
    };
}

#endif // !MANAGER_H
