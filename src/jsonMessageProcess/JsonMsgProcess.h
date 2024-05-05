#ifndef JSON_MESSAGE_PROCESS_H
#define JSON_MESSAGE_PROCESS_H

#include <string>
#include <map>
#include <asio.hpp>
#include <Json.h>
#include <memory>

namespace qls
{
    class JsonMessageProcessImpl;

    class JsonMessageProcess
    {
    public:
        JsonMessageProcess(long long user_id);
        ~JsonMessageProcess() = default;

        asio::awaitable<long long> getLocalUserID() const;
        asio::awaitable<qjson::JObject> processJsonMessage(const qjson::JObject& json);
        
    private:
        std::shared_ptr<JsonMessageProcessImpl> m_process;
    };
}

#endif // !JSON_MESSAGE_PROCESS_H