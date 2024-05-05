#include "dataPackage.h"

#include <stdexcept>
#include <format>

#include "networkEndinass.hpp"

namespace qls
{
    std::shared_ptr<DataPackage> DataPackage::makePackage(std::string_view data)
    {
        std::hash<std::string_view> hash;
        std::shared_ptr<DataPackage> package(reinterpret_cast<DataPackage*>(new char[sizeof(DataPackage) + data.size()] { 0 }),
            deleteDataPackage);
        package->length = int(sizeof(DataPackage) + data.size());
        std::memcpy(package->data, data.data(), data.size());
        package->verifyCode = hash(data);
        return package;
    }

    std::shared_ptr<DataPackage> DataPackage::stringToPackage(const std::string& data)
    {
        using namespace qls;

        // 数据包过小
        if (data.size() < sizeof(DataPackage)) throw std::logic_error("data is too small!");

        // 数据包length
        int size = 0;
        std::memcpy(&size, data.c_str(), sizeof(int));
        size = swapNetworkEndianness(size);

        // 数据包length与实际大小不符、length小于数据包默认大小、length非常大、数据包结尾不为2 * '\0' 报错处理
        if (size != data.size() || size < sizeof(DataPackage)) throw std::logic_error("data is invalid!");
        else if (size > INT32_MAX / 2) throw std::logic_error("data is too large!");
        else if (data[size_t(size - 1)] || data[size_t(size - 2)]) throw std::logic_error("data is invalid");

        std::shared_ptr<DataPackage> package(reinterpret_cast<DataPackage*>(new char[size] { 0 }),
            deleteDataPackage);
        std::memcpy(package.get(), data.c_str(), size);

        // 端序转换
        package->length = swapNetworkEndianness(package->length);
        package->requestID = swapNetworkEndianness(package->requestID);
        package->type = swapNetworkEndianness(package->type);
        package->sequence = swapNetworkEndianness(package->sequence);
        package->verifyCode = swapNetworkEndianness(package->verifyCode);

        std::hash<std::string_view> hash;
        size_t gethash = hash(package->getData());
        if (gethash != package->verifyCode) throw std::logic_error(std::format("hash is different, local hash: {}, pack hash: {}",
            gethash, package->verifyCode));

        return package;
    }

    std::string DataPackage::packageToString() noexcept
    {
        using namespace qls;

        size_t localLength = this->length;

        // 端序转换
        this->length = swapNetworkEndianness(this->length);
        this->requestID = swapNetworkEndianness(this->requestID);
        this->type = swapNetworkEndianness(this->type);
        this->sequence = swapNetworkEndianness(this->sequence);
        this->verifyCode = swapNetworkEndianness(this->verifyCode);

        std::string data;
        data.resize(localLength);
        std::memcpy(data.data(), this, localLength);
        return data;
    }

    size_t DataPackage::getPackageSize() noexcept
    {
        int size = 0;
        std::memcpy(&size, &(this->length), sizeof(int));
        return size_t(size);
    }

    size_t DataPackage::getDataSize() noexcept
    {
        int size = 0;
        std::memcpy(&size, &(this->length), sizeof(int));
        return size_t(size) - sizeof(DataPackage);
    }

    std::string DataPackage::getData()
    {
        std::string data;
        size_t size = this->getDataSize();
        data.resize(size);
        std::memcpy(data.data(), this->data, size);
        return data;
    }

    void DataPackage::deleteDataPackage(DataPackage* dp)
    {
        delete[] reinterpret_cast<char*>(dp);
    }
}