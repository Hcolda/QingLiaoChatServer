#ifndef SESSION_H
#define SESSION_H

#include <memory>
#include <string>
#include <string_view>

#include "network.h"
#include "option.hpp"
#include <groupid.hpp>
#include <userid.hpp>

namespace qls
{
    
struct SessionImpl;
class Session final
{
public:
    Session(Network& network);
    ~Session();

    void registerUser(std::string_view email, std::string_view password);
    void loginUser(std::string_view email, std::string_view password);

    void createFriendApplication(UserID userid);
    void applyFriendApplication(UserID userid);
    void rejectFriendApplication(UserID userid);

    void createGroupApplication(GroupID groupid);
    void applyGroupApplication(GroupID groupid, UserID userid);
    void rejectGroupApplication(GroupID groupid, UserID userid);

    void sendFriendMessage(UserID userid, std::string_view message);
    void sendGroupMessage(GroupID groupid, std::string_view message);

    void removeFriend(UserID userid);
    void leaveGroup(GroupID groupid);

private:
    std::unique_ptr<SessionImpl> m_impl;
};

} // namespace qls

#endif
