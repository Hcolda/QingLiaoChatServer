#ifndef JSON_MESSAGE_PROCESS_H
#define JSON_MESSAGE_PROCESS_H

#include <string>
#include <map>
#include <asio.hpp>
#include <Json.h>
#include <memory>

#include "socketFunctions.h"

namespace qls
{

class JsonMessageProcessImpl;

class JsonMessageProcess final
{
public:
    JsonMessageProcess(long long user_id);
    ~JsonMessageProcess();

    long long getLocalUserID() const;
    asio::awaitable<qjson::JObject> processJsonMessage(const qjson::JObject& json, const SocketService& sf);
    
private:
    std::unique_ptr<JsonMessageProcessImpl> m_process;
};

} // namespace qls

#endif // !JSON_MESSAGE_PROCESS_H