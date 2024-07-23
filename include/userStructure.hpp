#ifndef USER_STRUCTURE_H
#define USER_STRUCTURE_H

#include <string>

namespace qls
{

enum class DeviceType
{
    Unknown = 0,
    PersonalComputer,
    Phone,
    Web
};

struct UserVerificationStruct
{
    enum class VerificationType
    {
        Unknown = 0,
        Sent,
        Received
    };

    long long user_id = 0;
    /*
    * @brief 验证类型:
    * @brief {Unknown:  无状态}
    * @brief {Sent:     发出的申请}
    * @brief {Received: 接收的申请}
    */
    VerificationType verification_type = VerificationType::Unknown;
    bool has_message = false;
    std::string message;
};

}

#endif // !USER_STRUCTURE_H