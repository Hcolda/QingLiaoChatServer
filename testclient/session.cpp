#include "session.h"

#include <vector>
#include <iostream>
#include <Json.h>

namespace qls
{

struct SessionImpl
{
    Network& network;
    UserID user_id;
    bool has_login = false;
};

static qjson::JObject makeJsonFunctionDataPackage(std::string_view functionName,
    std::initializer_list<std::pair<std::string, qjson::JObject>> list)
{
    qjson::JObject json(qjson::JValueType::JDict);
    json["function"] = functionName;
    json["parameters"] = qjson::JObject(qjson::JValueType::JDict);
    for (auto [parameterName, parameterValue]: list)
    {
        json["parameters"][parameterName.c_str()] = parameterValue;
    }
    return json;
}

static qjson::JObject readJsonFunctionDataPackage(const std::shared_ptr<qls::DataPackage>& package)
{
    return qjson::JParser::fastParse(package->getData());
}

Session::Session(Network& network):
    m_impl(std::make_unique<SessionImpl>(network)) {}

Session::~Session() = default;

bool Session::registerUser(std::string_view email, std::string_view password, UserID& newUserID)
{
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("register", {{"email", email}, {"password", password}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    bool returnState = returnJson["state"].getString() == "success";
    if (returnState)
        newUserID = UserID(returnJson["user_id"].getInt());
    return returnState;
}

bool Session::loginUser(UserID user_id, std::string_view password)
{
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("login",
            {{"user_id", user_id.getOriginValue()}, {"password", password}, {"device", "PersonalComputer"}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    bool returnState = returnJson["state"].getString() == "success";
    if (returnState)
    {
        m_impl->user_id = user_id;
        m_impl->has_login = true;
    }
    return returnState;
}

bool Session::createFriendApplication(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("add_friend", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::applyFriendApplication(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("accept_friend_verification", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::rejectFriendApplication(UserID user_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("reject_friend_verification", {{"user_id", user_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::createGroupApplication(GroupID group_id)
{
    if (!m_impl->has_login)
        return false;
    auto returnPackage = m_impl->network.send_data_with_result_n_option(qjson::JWriter::fastWrite(
        makeJsonFunctionDataPackage("add_group", {{"group_id", group_id.getOriginValue()}})),
        [](std::shared_ptr<qls::DataPackage>& package){
            package->type = DataPackage::Text;
        }).get();
    auto returnJson = readJsonFunctionDataPackage(returnPackage);
    std::cout << returnJson["message"].getString() << '\n';
    return returnJson["state"].getString() == "success";
}

bool Session::applyGroupApplication(GroupID groupid, UserID userid)
{
    if (!m_impl->has_login)
        return false;
    return false;
}

bool Session::rejectGroupApplication(GroupID groupid, UserID userid)
{
    if (!m_impl->has_login)
        return false;
    return false;
}

bool Session::sendFriendMessage(UserID userid, std::string_view message)
{
    if (!m_impl->has_login)
        return false;
    return false;
}

bool Session::sendGroupMessage(GroupID groupid, std::string_view message)
{
    if (!m_impl->has_login)
        return false;
    return false;
}

bool Session::removeFriend(UserID userid)
{
    if (!m_impl->has_login)
        return false;
    return false;
}

bool Session::leaveGroup(GroupID groupid)
{
    if (!m_impl->has_login)
        return false;
    return false;
}

} // namespace qls
