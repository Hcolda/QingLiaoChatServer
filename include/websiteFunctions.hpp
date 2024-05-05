#ifndef WEBSITE_FUNCTIONS_HPP
#define WEBSITE_FUNCTIONS_HPP

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
        static constexpr char serverUrl[] = "https://account.hcolda.com";
        static constexpr char serverUUID[] = "00000000-0000-0000-0000-000000000000";
        static constexpr char serverToken[] = "Default";
        /*
        * @brief 网站状态获取结构体
        */
        struct WebState
        {
            double APIVer;
            int MaxClientVer;
            int MinClientVer;
        };
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
            static httplib::Client client(serverUrl);
            httplib::Params param;
            param.insert({{ "ServerUUID", serverUUID }, { "ServerToken", serverToken }, { "ConnUUID", uuid}});
            auto result = client.Post("/server.php?type=getaes", httplib::Headers(), param);

            if (!result) throw std::runtime_error("connection of website is down");

            qjson::JObject json = qjson::JParser::fastParse(result->body);
            if (json["state"].getString() != "success")
                throw std::runtime_error(std::format(
                    "state is\"{}\"\n"
                    "type is \"{}\"\n"
                    "message is \"{}\"",
                    json["state"].getString(),
                    json["type"].getString(),
                    json["message"].getString()
                ));

            return { json["key"].getString(), json["iv"].getString() };
        }


        /*
        * @brief 获取用户唯一id
        * @param uuid
        * @return user id
        */
        static long long getUserID(const std::string& uuid)
        {
            static httplib::Client client(serverUrl);

            httplib::Params param;
            param.insert({ { "uuid", uuid} });
            auto result = client.Post("/api.php?type=getID", httplib::Headers(), param);

            if (!result) throw std::runtime_error("connection of website is down");

            qjson::JObject json = qjson::JParser::fastParse(result->body);
            if (json["state"].getString() != "success") throw std::runtime_error("state is not 'success'");
            return json["user_id"].getInt();
        }

        /*
        * @brief 获取服务端状态
        * @param NONE
        * @return APIVer MaxClientVer MinClientVer
        */
        static struct WebState getServerState()
        {
            static httplib::Client client(serverUrl);
            WebState webState;
            auto result = client.Post("/api.php?type=state", httplib::Headers());

            if (!result) throw std::runtime_error("connection of website is down");

            qjson::JObject json = qjson::JParser::fastParse(result->body);
            if (json["state"].getString() != "success") throw std::runtime_error("state is not 'success'");

            webState.APIVer         = double(json["APIVer"].getDouble());
            webState.MinClientVer   = int(json["version"][1].getInt());
            webState.MaxClientVer   = int(json["version"][0].getInt());
            return webState;
        }
    };
}

#endif // !WEBSITE_FUNCTIONS_HPP
