#ifndef DATA_PACKAGE_H
#define DATA_PACKAGE_H

#include <string>
#include <memory>
#include <string_view>

namespace qls
{
    /*
    * @brief 数据包
    */
    class DataPackage
    {
    private:
#pragma pack(1)
        int                 length = 0;
    public:
        long long           requestID = 0;
        int                 type = 0;
        int                 sequence = -1;
    private:
        unsigned long long  verifyCode = 0;
        char                data[2]{ 0 };
#pragma pack()

    public:
        DataPackage() = delete;
        ~DataPackage() = default;
        DataPackage(const DataPackage&) = delete;
        DataPackage(DataPackage&& dp) = delete;

        /*
        * @brief 制作数据包
        * @param data 数据包中需要存的二进制数据
        * @return 带自动回收的数据包
        */
        static std::shared_ptr<DataPackage> makePackage(std::string_view data);

        /*
        * @brief 从string中加载数据包
        * @param data 数据包中需要存的二进制数据
        * @return 带自动回收的数据包
        */
        static std::shared_ptr<DataPackage> stringToPackage(const std::string& data);

        /*
        * @brief 将数据包转换为二进制格式数据包
        * @param dp DataPackage
        * @return 二进制格式数据包
        */
        std::string packageToString() noexcept;

        /*
        * @brief 获取数据包大小
        * @return size 数据包大小
        */
        size_t getPackageSize() noexcept;

        /*
        * @brief 获取包中二进制数据大小
        * @return size 二进制数据大小
        */
        size_t getDataSize() noexcept;

        /*
        * @brief 获取包中二进制数据
        * @return string 二进制数据
        */
        std::string getData();

        static void deleteDataPackage(DataPackage* dp);
    };
}

#endif // !DATA_PACKAGE_H
