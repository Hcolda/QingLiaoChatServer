#ifndef RETURN_STATE_MESSAGE_HPP
#define RETURN_STATE_MESSAGE_HPP

#include <string>
#include <Json.h>

namespace qls
{
    inline qjson::JObject makeMessage(const std::string& state, const std::string& msg)
    {
        qjson::JObject json;

        json["state"] = state;
        json["message"] = msg;

        return json;
    }

    inline qjson::JObject makeErrorMessage(const std::string& msg)
    {
        return makeMessage("error", msg);
    }

    inline qjson::JObject makeSuccessMessage(const std::string& msg)
    {
        return makeMessage("success", msg);
    }
}

#endif // !RETURN_STATE_MESSAGE_HPP
