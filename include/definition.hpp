#ifndef DEFINITION_HPP
#define DEFINITION_HPP

#ifdef MSC_VER
    #include <format>
    #include <filesystem>
    #ifdef _HAS_CXX23
        #include <stacktrace>
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
    #else
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__)
    #endif // !_HAS_CXX23

#else // !MSC_VER
    #include <format>
    #include <filesystem>
    #include <stacktrace>
    #if defined(__cplusplus) && __cplusplus >= 202011L && defined(__cpp_lib_stacktrace)
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
    #else
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__)
    #endif // !_HAS_CXX23
#endif // !MSC_VER

#include <unordered_map>

namespace qls
{

enum class DeviceType
{
    Unknown = 0,
    PersonalComputer,
    Phone,
    Web
};

struct PrivateRoomIDStruct
{
    long long user_id_1;
    long long user_id_2;
    
    friend bool operator==(const PrivateRoomIDStruct& a, const PrivateRoomIDStruct& b)
    {
        return (a.user_id_1 == b.user_id_1 && a.user_id_2 == b.user_id_2) ||
            (a.user_id_2 == b.user_id_1 && a.user_id_1 == b.user_id_2);
    }

    friend bool operator!=(const PrivateRoomIDStruct& a, const PrivateRoomIDStruct& b)
    {
        return !(a == b);
    }
};

class PrivateRoomIDStructHasher
{
public:
    PrivateRoomIDStructHasher() = default;
    ~PrivateRoomIDStructHasher() = default;

    template<class T, class Y =
        std::enable_if_t<std::is_same_v<
        std::remove_const_t<std::remove_reference_t<T>>,
        PrivateRoomIDStruct>>>
    size_t operator()(T&& s) const
    {
        std::hash<long long> hasher;
        return hasher(s.user_id_1) * hasher(s.user_id_2);
    }
};

struct GroupVerificationStruct
{
    long long group_id;
    long long user_id;

    friend bool operator==(const GroupVerificationStruct& a, const GroupVerificationStruct& b)
    {
        return a.group_id == b.group_id && a.user_id == b.user_id;
    }

    friend bool operator!=(const GroupVerificationStruct& a, const GroupVerificationStruct& b)
    {
        return !(a == b);
    }
};

class GroupVerificationStructHasher
{
public:
    GroupVerificationStructHasher() = default;
    ~GroupVerificationStructHasher() = default;

    template<class T, class Y =
        std::enable_if_t<std::is_same_v<
        std::remove_const_t<std::remove_reference_t<T>>,
        GroupVerificationStruct>>>
    size_t operator()(T&& g) const
    {
        std::hash<long long> hasher;
        return hasher(g.group_id) * hasher(g.user_id);
    }
};

}

#endif // !DEFINITION_HPP