#ifndef DATA_PACKAGE_H
#define DATA_PACKAGE_H

#include <string>
#include <memory>
#include <string_view>

namespace qls
{
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

        /// @brief Make data package
        /// @param data Original data would be stored in the data package
        /// @return Shared pointer of data package
        static std::shared_ptr<DataPackage> makePackage(std::string_view data);

        /// @brief Load data package from binary data
        /// @param data Binary data
        /// @return Shared pointer of data package
        static std::shared_ptr<DataPackage> stringToPackage(const std::string& data);

        /// @brief Generate binary data for this data package
        /// @return binary data
        std::string packageToString() noexcept;

        /// @brief Get the size of this data package
        /// @return Size of this data package
        size_t getPackageSize() noexcept;

        /// @brief Get the size of the original data in this data package
        /// @return Size of the original data in this data package
        size_t getDataSize() noexcept;

        /// @brief Get the original data in this data package
        /// @return original data in this data package
        std::string getData();

        static void deleteDataPackage(DataPackage* dp);
    };
}

#endif // !DATA_PACKAGE_H
