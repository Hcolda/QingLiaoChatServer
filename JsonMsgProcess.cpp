#include "JsonMsgProcess.h"

#include <format>

namespace qls
{
    JsonMessageProcess::JsonMessageProcess(long long user_id) :
        m_user_id(user_id)
    {
        
    }

    qjson::JObject JsonMessageProcess::makeErrorMessage(const std::string& msg)
    {
        qjson::JObject json;

        json["state"] = "error";
        json["error_message"] = msg;

        return json;
    }

}
