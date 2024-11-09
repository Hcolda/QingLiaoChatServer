#include "JsonMsgProcessCommand.h"

#include <string>
#include <format>
#include <unordered_set>
#include <logger.hpp>
#include "manager.h"
#include "regexMatch.hpp"
#include "returnStateMessage.hpp"
#include "definition.hpp"
#include "groupid.hpp"
#include "userid.hpp"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

qjson::JObject qls::LoginCommand::execute(opt::Option opt, bool& success)
{
    // UserID user_id = UserID(opt.get_int("user_id"));
    // std::string password = opt.get_string("password");
    // std::string device = opt.get_string("device");
    // if (!serverManager.hasUser(user_id))
    //     return makeErrorMessage("The user ID or password is wrong!");
    
    // auto user = serverManager.getUser(user_id);
    
    // if (user->isUserPassword(password)) {
    //     // check device type
    //     if (device == "PersonalComputer")
    //         serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::PersonalComputer);
    //     else if (device == "Phone")
    //         serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::Phone);
    //     else if (device == "Web")
    //         serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::Web);
    //     else
    //         serverManager.modifyUserOfSocket(sf.get_socket_ptr(), user_id, DeviceType::Unknown);

    //     auto returnJson = makeSuccessMessage("Successfully logged in!");
    //     std::unique_lock<std::shared_mutex> local_unique_lock(m_user_id_mutex);
    //     this->m_user_id = user_id;

        
    //     serverLogger.debug("User ", user_id.getOriginValue(), " logged into the server");

    //     return returnJson;
    // }
    // else return makeErrorMessage("The user ID or password is wrong!");
    return qjson::JObject();
}

qjson::JObject qls::RegisterCommand::execute(opt::Option opt, bool &success)
{
    return qjson::JObject();
}
