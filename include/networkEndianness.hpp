#ifndef NETWORK_ENDIANNESS_HPP
#define NETWORK_ENDIANNESS_HPP

#include <concepts>

namespace qls
{

/// @brief Determine if the system is big endianness
/// @return True if it is big endianness
constexpr inline bool isBigEndianness() noexcept
{
    union u_data
    {
        unsigned char   a;
        unsigned int    b;
    } data;

    data.b = 0x12345678;

    return data.a == 0x12;
}

/// @brief Convert number of endianness
/// @tparam T Type of integral
/// @param value Integral of the other endianness
/// @return Integral of new endianness
template<typename T>
    requires std::integral<T>
constexpr inline T swapEndianness(T value) noexcept {
    T result = 0;
    for (size_t i = 0; i < sizeof(value); ++i) {
        result = (result << 8) | ((value >> (8 * i)) & 0xFF);
    }
    return result;
}

/// @brief Convert if the network endianness is different from local system
/// @tparam T Type of integral
/// @param value Integral of the other endianness
/// @return Integral of new endianness
template<typename T>
    requires std::integral<T>
constexpr inline T swapNetworkEndianness(T value) noexcept
{
    if (!isBigEndianness())
        return swapEndianness(value);
    else
        return value;
}

} // namespace qls

#endif // !NETWORK_ENDIANNESS_HPP
