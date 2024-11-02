#include "dataPackage.h"

#include <stdexcept>
#include <format>
#include <cstring>

#include "networkEndianness.hpp"
#include "qls_error.h"

namespace qls
{

std::shared_ptr<DataPackage> DataPackage::makePackage(std::string_view data)
{
    std::hash<std::string_view> hash;
    std::shared_ptr<DataPackage> package(reinterpret_cast<DataPackage*>(
        new char[sizeof(DataPackage) + data.size()] { 0 }),
        deleteDataPackage);
    package->length = int(sizeof(DataPackage) + data.size());
    std::memcpy(package->data, data.data(), data.size());
    package->verifyCode = hash(data);
    return package;
}

std::shared_ptr<DataPackage> DataPackage::stringToPackage(const std::string& data)
{
    using namespace qls;

    // Data package is too small
    if (data.size() < sizeof(DataPackage)) throw std::system_error(qls_errc::data_too_small);

    // Data package length
    int size = 0;
    std::memcpy(&size, data.c_str(), sizeof(int));
    size = swapNetworkEndianness(size);

    // Error handling if data package length does not match actual size,
    // length is smaller than the default package size, length is very large,
    // or the package ends not with 2 * '\0'
    if (size != data.size() || size < sizeof(DataPackage))
        throw std::system_error(qls_errc::invalid_data);
    else if (size > INT32_MAX / 2)
        throw std::system_error(qls_errc::data_too_large);
    else if (data[size_t(size - 1)] || data[size_t(size - 2)])
        throw std::system_error(qls_errc::invalid_data);

    std::shared_ptr<DataPackage> package(reinterpret_cast<DataPackage*>(new char[size] { 0 }),
        deleteDataPackage);
    std::memcpy(package.get(), data.c_str(), size);

    // Endianness conversion
    package->length = swapNetworkEndianness(package->length);
    package->requestID = swapNetworkEndianness(package->requestID);
    package->type = swapNetworkEndianness(package->type);
    package->sequence = swapNetworkEndianness(package->sequence);
    package->verifyCode = swapNetworkEndianness(package->verifyCode);

    std::hash<std::string_view> hash;
    size_t gethash = hash(package->getData());
    if (gethash != package->verifyCode)
        throw std::system_error(make_error_code(qls_errc::hash_mismatched),
            std::format("hash is different, local hash: {}, pack hash: {}",
            gethash, package->verifyCode));

    return package;
}

std::string DataPackage::packageToString() noexcept
{
    using namespace qls;

    size_t localLength = this->length;

    // Endianness conversion
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

} // namespace qls
