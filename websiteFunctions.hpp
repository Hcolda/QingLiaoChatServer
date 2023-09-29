#pragma once

#include <string>
#include <string_view>
#include <httplib.h>
#include <Json.h>

namespace qls
{
    /*
    * @brief 用来访问网站的一系列集合
    */
    class WebFunction
    {
    public:
        WebFunction() = default;
        ~WebFunction() = default;

        int connectionState()
        {
            return 1;
        }

        /*
        * @brief 获取aes密钥
        * @param uuid
        * @return key, iv
        */
        static std::pair<std::string, std::string> getAESKey(const std::string& uuid)
        {
            static httplib::Client client("https://account.hcolda.com");
            httplib::Params param;
            param.insert({{ "serverid", "" }, { "serverkey", "" }, { "uuid", uuid}});
            auto result = client.Post("/api.php?type=aeskey", httplib::Headers(), param);
            if (!result) throw std::runtime_error("connection of website is down");
            qjson::JObject json = qjson::JParser::fastParse(result->body);
            if (json["state"].getString() != "success") throw std::runtime_error("state is not 'success'");
            return { json["key"].getString(), json["iv"].getString() };
        }


        /*
        * @brief 获取用户唯一id
        * @param uuid
        * @return user id
        */
        static long long getUserID(const std::string& uuid)
        {
            static httplib::Client client("https://account.hcolda.com");
            httplib::Params param;
            param.insert({ { "uuid", uuid} });
            auto result = client.Post("/api.php?type=getID", httplib::Headers(), param);
            if (!result) throw std::runtime_error("connection of website is down");
            qjson::JObject json = qjson::JParser::fastParse(result->body);
            if (json["state"].getString() != "success") throw std::runtime_error("state is not 'success'");
            return json["user_id"].getInt();
        }
    };
}
