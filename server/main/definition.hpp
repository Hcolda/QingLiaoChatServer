#ifndef DEFINITION_HPP
#define DEFINITION_HPP

#ifdef _MSC_VER
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

#else // !_MSC_VER
    #include <format>
    #include <filesystem>
    #include <stacktrace>
    #if defined(__cplusplus) && __cplusplus >= 202011L && defined(__cpp_lib_stacktrace)
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
    #else
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__)
    #endif // !defined(__cplusplus) && __cplusplus >= 202011L && defined(__cpp_lib_stacktrace)
#endif // !_MSC_VER

#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>

#include "groupid.hpp"
#include "userid.hpp"

namespace qls
{

enum class DeviceType
{
    Unknown = 0,
    PersonalComputer,
    Phone,
    Web
};

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
};

struct PrivateRoomIDStruct
{
    UserID user_id_1;
    UserID user_id_2;
    
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
        std::enable_if_t<std::is_same_v<std::decay_t<T>, PrivateRoomIDStruct>>>
    size_t operator()(T&& s) const
    {
        std::hash<long long> hasher;
        return hasher(s.user_id_1.getOriginValue()) ^ hasher(s.user_id_2.getOriginValue());
    }
};

struct GroupVerificationStruct
{
    GroupID group_id;
    UserID user_id;

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
        std::enable_if_t<std::is_same_v<std::decay_t<T>, GroupVerificationStruct>>>
    size_t operator()(T&& g) const
    {
        std::hash<long long> hasher;
        return hasher(g.group_id.getOriginValue()) ^ hasher(g.user_id.getOriginValue());
    }
};

} // namespace qls

#endif // !DEFINITION_HPP