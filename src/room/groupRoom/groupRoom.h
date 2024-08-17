#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include <memory>
#include <chrono>
#include <vector>
#include <shared_mutex>
#include <unordered_map>
#include <asio.hpp>

#include "room.h"
#include "groupPermission.h"

namespace qls
{

struct GroupRoomImpl;

class GroupRoom final : public qls::BaseRoom
{
public:
    struct UserDataStructure
    {
        std::string nickname;
        long long groupLevel = 1;
    };

    GroupRoom(long long group_id, long long administrator, bool is_create);
    GroupRoom(const GroupRoom&) = delete;
    GroupRoom(GroupRoom&&) = delete;
    ~GroupRoom();

    bool addMember(long long user_id);
    bool removeMember(long long user_id);
    
    void sendMessage(long long sender_user_id, const std::string& message);
    void sendTipMessage(long long sender_user_id, const std::string& message);
    void sendUserTipMessage(long long sender_user_id, const std::string& message, long long receiver_user_id);
    void getMessage(
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to);

    bool                                    hasUser(long long user_id) const;
    std::unordered_map<long long,
        UserDataStructure>                  getUserList() const;
    std::string                             getUserNickname(long long user_id) const;
    long long                               getUserGroupLevel(long long user_id) const;
    std::unordered_map<long long,
        PermissionType>    getUserPermissionList() const;
    long long                               getAdministrator() const;
    long long                               getGroupID() const;
    std::vector<long long>                  getDefaultUserList() const;
    std::vector<long long>                  getOperatorList() const;
    
    bool muteUser(long long executorId, long long user_id, const std::chrono::minutes& mins);
    bool unmuteUser(long long executorId, long long user_id);
    bool kickUser(long long executorId, long long user_id);
    bool addOperator(long long executorId, long long user_id);
    bool removeOperator(long long executorId, long long user_id);
    void setAdministrator(long long user_id);

    void removeThisRoom();
    bool canBeUsed() const;

private:
    std::unique_ptr<GroupRoomImpl> m_impl;
};

} // namespace qls

#endif // !GROUP_ROOM_H
