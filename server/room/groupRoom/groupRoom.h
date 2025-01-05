#ifndef GROUP_ROOM_H
#define GROUP_ROOM_H

#include <atomic>
#include <memory>
#include <chrono>
#include <vector>
#include <shared_mutex>
#include <unordered_map>
#include <asio.hpp>

#include "qls_error.h"
#include "userid.hpp"
#include "groupid.hpp"
#include "room.h"
#include "groupPermission.h"
#include "groupUserLevel.hpp"

namespace qls
{

struct GroupRoomImpl;
struct GroupRoomImplDeleter
{
    void operator()(GroupRoomImpl* gri);
};

class GroupRoom: public TextDataRoom
{
public:
    struct UserDataStructure
    {
        std::string nickname;
        UserLevel<1, 100> level;
    };

    GroupRoom(GroupID group_id, UserID administrator, bool is_create);
    GroupRoom(const GroupRoom&) = delete;
    GroupRoom(GroupRoom&&) = delete;
    ~GroupRoom() noexcept;

    bool addMember(UserID user_id);
    bool hasMember(UserID user_id) const;
    bool removeMember(UserID user_id);
    
    void sendMessage(UserID sender_user_id, std::string_view message);
    void sendTipMessage(UserID sender_user_id, std::string_view message);
    void sendUserTipMessage(UserID sender_user_id, std::string_view, UserID receiver_user_id);
    void getMessage(
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& from,
        const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& to);

    bool                                    hasUser(UserID user_id) const;
    std::unordered_map<UserID,
        UserDataStructure>                  getUserList() const;
    std::string                             getUserNickname(UserID user_id) const;
    long long                               getUserGroupLevel(UserID user_id) const;
    std::unordered_map<UserID, PermissionType>
                                            getUserPermissionList() const;
    UserID                                  getAdministrator() const;
    GroupID                                 getGroupID() const;
    std::vector<UserID>                     getDefaultUserList() const;
    std::vector<UserID>                     getOperatorList() const;
    
    bool muteUser(UserID executor_id, UserID user_id, const std::chrono::minutes& mins);
    bool unmuteUser(UserID executor_id, UserID user_id);
    bool kickUser(UserID executor_id, UserID user_id);
    bool addOperator(UserID executor_id, UserID user_id);
    bool removeOperator(UserID executor_id, UserID user_id);
    void setAdministrator(UserID user_id);

    void removeThisRoom();
    bool canBeUsed() const;

private:
    std::unique_ptr<GroupRoomImpl, GroupRoomImplDeleter> m_impl;
};

} // namespace qls

#endif // !GROUP_ROOM_H
