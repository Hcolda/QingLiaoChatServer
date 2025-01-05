#ifndef ROOM_H
#define ROOM_H

#include <memory>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <memory_resource>

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
struct BaseRoomImplDeleter
{
    std::pmr::memory_resource* memory_resource;
    void operator()(BaseRoomImpl*) noexcept;
};

class BaseRoom
{
public:
    BaseRoom(std::pmr::memory_resource* mr);
    BaseRoom(const BaseRoom&) = delete;
    BaseRoom(BaseRoom&&) = delete;
    virtual ~BaseRoom() noexcept;

    BaseRoom& operator=(const BaseRoom&) = delete;
    BaseRoom& operator=(BaseRoom&&) = delete;

    virtual void joinRoom(UserID user_id);
    virtual bool hasUser(UserID user_id) const;
    virtual void leaveRoom(UserID user_id);

    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);

private:
    std::unique_ptr<BaseRoomImpl, BaseRoomImplDeleter> m_impl;
};

class TextDataRoom: public BaseRoom
{
public:
    TextDataRoom(std::pmr::memory_resource* mr):
        BaseRoom(mr) {}
    virtual ~TextDataRoom() noexcept = default;

protected:
    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);
};

} // namespace qls

#endif // !ROOM_H
