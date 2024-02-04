#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <string_view>

namespace qls
{
    /*
    * @brief 数据包处理类
    */
    class Package
    {
    public:
        Package() = default;
        ~Package() = default;

        Package(const Package&) = delete;
        Package(Package&&) = delete;

        Package& operator =(const Package&) = delete;
        Package& operator =(Package&&) = delete;

        /*
        * @brief 写入数据到类中
        * @param data 二进制数据
        */
        void write(std::string_view data);

        /*
        * @brief 是否可以读取
        * @return true 是, false 否
        */
        bool canRead() const;

        /*
        * @brief 第一个数据包的长度
        * @return size 第一个数据包的长度
        */
        size_t firstMsgLength() const;

        /*
        * @brief 读取数据包
        * @return 返回数据包
        */
        std::string read();

        /*
        * @brief 读取类中buffer数据
        * @return string buffer
        */
        const std::string& readBuffer() const;

        /*
        * @brief 设置buffer
        * @param buffer
        */
        void setBuffer(const std::string& b);

        /*
        * @brief 制造数据包
        * @param 二进制数据
        * @return 经过数据包包装的二进制数据
        */
        static std::string makePackage(std::string_view data);

    private:
        std::string m_buffer;
    };
}

#endif // !PACKAGE_H
