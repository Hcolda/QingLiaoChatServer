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
        WebFunction() :
            m_client("https://testapi.hcolda.com") {}
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
        std::pair<std::string, std::string> getAESKey(const std::string& uuid)
        {
            httplib::Params param;
            param.insert({{ "serverid", "" }, { "serverkey", "" }, { "uuid", uuid}});
            auto result = m_client.Post("/api.php?type=server&commend=aeskey", httplib::Headers(), param);
            if (!result) throw std::runtime_error("connection of website is down");
            qjson::JObject json = qjson::JParser::fastParse(result->body);
            if (json["state"].getString() != "success") throw std::runtime_error("state is not 'success'");
            return { json["key"].getString(), json["iv"].getString() };
        }

    private:
        httplib::Client m_client;
    };
}
