#ifndef REGEX_MATCH_HPP
#define REGEX_MATCH_HPP

#include <regex>
#include <string>

namespace qls
{
    class RegexMatch
    {
    public:
        RegexMatch() = default;
        ~RegexMatch() = default;

        static bool emailMatch(const std::string& email)
        {
            static std::regex re(R"((\w+\.)*\w+@(\w+\.)+[A-Za-z]+)", std::regex::optimize);
            std::smatch results;
            return std::regex_match(email, results, re);
        }

        static bool ipAddressMatch(const std::string& ip)
        {
            static std::regex re(R"((((\d{1,2})|(1\d{2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{2})|(2[0-4]\d)|(25[0-5])))",
                std::regex::optimize);
            std::smatch results;
            return std::regex_match(ip, results, re);
        }

        static bool phoneMatch(const std::string& phone)
        {
            static std::regex re(R"(\d{11})",
                std::regex::optimize);
            std::smatch results;
            return std::regex_match(phone, results, re);
        }
    };
}

#endif // !REGEX_MATCH_HPP
