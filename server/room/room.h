#ifndef ROOM_H
#define ROOM_H

#include <memory>
#include <functional>
#include <stdexcept>
#include <string_view>

#include "userid.hpp"
#include "user.h"

namespace qls
{

enum class MessageType
{
    NOMAL_MESSAGE = 0,
    TIP_MESSAGE
};

struct MessageStructure
{
    UserID user_id = UserID(-1ll);
    std::string message;
    MessageType type;
};

struct BaseRoomImpl;

class BaseRoom
{
public:
    BaseRoom() = default;
    BaseRoom(const BaseRoom&) = delete;
    BaseRoom(BaseRoom&&) = delete;
    virtual ~BaseRoom() noexcept = default;

    BaseRoom& operator=(const BaseRoom&) = delete;
    BaseRoom& operator=(BaseRoom&&) = delete;

    virtual bool joinRoom(UserID user_id);
    virtual bool hasUser(UserID user_id) const;
    virtual bool leaveRoom(UserID user_id);

    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);

private:
    std::unordered_set<UserID>  m_user_set;
    mutable std::shared_mutex   m_user_set_mutex;
};

class TextDataRoom: public BaseRoom
{
public:
    TextDataRoom() = default;
    virtual ~TextDataRoom() noexcept = default;

protected:
    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);
};

} // namespace qls

#endif // !ROOM_H
