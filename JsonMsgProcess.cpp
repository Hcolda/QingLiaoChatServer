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

    qjson::JObject JsonMessageProcess::makeErrorMessage(const std::string& state, const std::string& msg)
    {
        qjson::JObject json;

        json["state"] = state;
        json["error_message"] = msg;

        return json;
    }

    qjson::JObject JsonMessageProcess::getUserPublicInfo(long long user_id)
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::processJsonMessage(const qjson::JObject& json)
    {
        try
        {
            std::string function_name = json["function"].getString();
            const qjson::JObject& param = json["param"];
            if (function_name == "")
            {
                /*
                * 此处代码不完善，需要完善
                */
            }

            return qjson::JObject();
        }
        catch (const std::exception& e)
        {
            return makeErrorMessage(e.what());
        }
    }

    qjson::JObject JsonMessageProcess::addFriend(long long friend_id)
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::getFriendList()
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::addGroup(long long group_id)
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::getGroupList()
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::sendFriendMessage(long long friend_id, const std::string& msg)
    {
        return qjson::JObject();
    }

    qjson::JObject JsonMessageProcess::sendGroupMessage(long long group_id, const std::string& msg)
    {
        return qjson::JObject();
    }

}
