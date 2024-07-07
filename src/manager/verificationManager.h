#ifndef VERIFICATION_MANAGER_H
#define VERIFICATION_MANAGER_H

#include <unordered_map>
#include <shared_mutex>

#include "friendRoomVerification.h"
#include "groupRoomVerification.h"
#include "structHasher.h"

namespace qls
{
    class VerificationManager
    {
    public:
        VerificationManager() = default;
        ~VerificationManager() = default;

        void init();

        void addFriendRoomVerification(long long user_id_1, long long user_id_2);
        bool hasFriendRoomVerification(long long user_id_1, long long user_id_2) const;
        bool setFriendVerified(long long user_id_1, long long user_id_2, long long user_id, bool is_verified);
        void removeFriendRoomVerification(long long user_id_1, long long user_id_2);

        void addGroupRoomVerification(long long group_id, long long user_id);
        bool hasGroupRoomVerification(long long group_id, long long user_id) const;
        bool setGroupRoomGroupVerified(long long group_id, long long user_id, bool is_verified);
        bool setGroupRoomUserVerified(long long group_id, long long user_id, bool is_verified);
        void removeGroupRoomVerification(long long group_id, long long user_id);

    private:
        // 用户添加用户申请表
        std::unordered_map<PrivateRoomIDStruct,
            qls::FriendRoomVerification,
            PrivateRoomIDStructHasher>          m_FriendRoomVerification_map;
        mutable std::shared_mutex               m_FriendRoomVerification_map_mutex;

        // 用户添加群聊申请表
        std::unordered_map<GroupVerificationStruct,
            qls::GroupRoomVerification,
            GroupVerificationStructHasher>      m_GroupVerification_map;
        mutable std::shared_mutex               m_GroupVerification_map_mutex;
    };
}

#endif // ! VERIFICATION_MANAGER_H