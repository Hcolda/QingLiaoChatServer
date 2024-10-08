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
    BaseRoom();
    virtual ~BaseRoom();

    virtual bool joinRoom(UserID user_id, const std::shared_ptr<User>& user_ptr);
    virtual bool hasUser(UserID user_id) const;
    virtual bool leaveRoom(UserID user_id);

    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);

private:
    std::unique_ptr<BaseRoomImpl> m_impl;
};

class TextDataRoom: public BaseRoom
{
public:
    TextDataRoom() = default;
    virtual ~TextDataRoom() = default;

protected:
    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);
};

} // namespace qls

#endif // !ROOM_H
