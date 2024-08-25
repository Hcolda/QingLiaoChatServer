#ifndef ROOM_H
#define ROOM_H

#include <memory>
#include <functional>
#include <stdexcept>
#include <string_view>

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
    long long user_id = -1ll;
    std::string message;
    MessageType type;
};

struct BaseRoomImpl;

class BaseRoom
{
public:
    BaseRoom();
    virtual ~BaseRoom();

    virtual bool joinRoom(long long user_id, const std::shared_ptr<User>& user_ptr);
    virtual bool hasUser(long long user_id) const;
    virtual bool leaveRoom(long long user_id);

    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, long long user_id);

private:
    std::unique_ptr<BaseRoomImpl> m_impl;
};

class ChattingRoom: public BaseRoom
{
public:
    ChattingRoom() = default;
    virtual ~ChattingRoom() = default;

protected:
    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, long long user_id);
};

} // namespace qls

#endif // !ROOM_H
