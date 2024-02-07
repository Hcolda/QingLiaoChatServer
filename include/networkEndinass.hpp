#ifndef NETWORK_ENDINASS_HPP
#define NETWORK_ENDINASS_HPP

#include <concepts>

namespace qls
{
    /*
    * @brief 判断本地序是否为大端序
    * @return true 为大端序 | false 为小端序
    */
    constexpr inline bool isBigEndianness()
    {
        union u_data
        {
            unsigned char   a;
            unsigned int    b;
        } data;

        data.b = 0x12345678;

        return data.a == 0x12;
    }

    /*
    * @brief 端序转换
    * @param value 数据 (整数型)
    * @return 转换端序后的数据
    */
    template<typename T>
        requires std::integral<T>
    constexpr inline T swapEndianness(T value) {
        T result = 0;
        for (size_t i = 0; i < sizeof(value); ++i) {
            result = (result << 8) | ((value >> (8 * i)) & 0xFF);
        }
        return result;
    }

    /*
    * @brief 本地序与网络序互转
    * @param value 数据 (整数型)
    * @return 转换端序后的数据
    */
    template<typename T>
        requires std::integral<T>
    constexpr inline T swapNetworkEndianness(T value)
    {
        if (!isBigEndianness())
            return swapEndianness(value);
        else
            return value;
    }
}

#endif // !NETWORK_ENDINASS_HPP
