#include "session.h"

namespace qls
{

struct SessionImpl
{
    Network& network;
};

Session::Session(Network& network):
    m_impl(std::make_unique<SessionImpl>(network))
{
}

Session::~Session() = default;

void Session::registerUser(std::string_view email, std::string_view password)
{
}

void Session::loginUser(std::string_view email, std::string_view password)
{
}

void Session::createFriendApplication(UserID userid)
{
}

void Session::applyFriendApplication(UserID userid)
{
}

void Session::rejectFriendApplication(UserID userid)
{
}

void Session::createGroupApplication(GroupID groupid)
{
}

void Session::applyGroupApplication(GroupID groupid, UserID userid)
{
}

void Session::rejectGroupApplication(GroupID groupid, UserID userid)
{
}

void Session::sendFriendMessage(UserID userid, std::string_view message)
{
}

void Session::sendGroupMessage(GroupID groupid, std::string_view message)
{
}

void Session::removeFriend(UserID userid)
{
}

void Session::leaveGroup(GroupID groupid)
{
}

} // namespace qls
