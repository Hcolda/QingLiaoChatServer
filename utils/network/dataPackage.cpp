#include "dataPackage.h"

#include <stdexcept>
#include <format>
#include <cstring>
#include <memory_resource>
#include <functional>

#include "networkEndianness.hpp"
#include "qls_error.h"

namespace qls
{

static std::pmr::synchronized_pool_resource sync_pool;

std::shared_ptr<DataPackage> DataPackage::makePackage(std::string_view data)
{
    std::hash<std::string_view> hash;
    const int lenth = static_cast<int>(sizeof(DataPackage) + data.size());
    void* mem = sync_pool.allocate(lenth);
    std::memset(mem, 0, lenth);
    std::shared_ptr<DataPackage> package(static_cast<DataPackage*>(mem),
        [lenth](DataPackage* dp) {
            sync_pool.deallocate(dp, static_cast<size_t>(lenth));
        });
    package->length = lenth;
    std::memcpy(package->data, data.data(), data.size());
    package->verifyCode = hash(data);
    return package;
}

std::shared_ptr<DataPackage> DataPackage::stringToPackage(std::string_view data)
{
    using namespace qls;

    // Data package is too small
    if (data.size() < sizeof(DataPackage)) throw std::system_error(qls_errc::data_too_small);

    // Data package length
    int size = 0;
    std::memcpy(&size, data.data(), sizeof(int));
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

    void* mem = sync_pool.allocate(size);
    std::memset(mem, 0, size);
    std::shared_ptr<DataPackage> package(static_cast<DataPackage*>(mem),
        [lenth = size](DataPackage* dp) {
            sync_pool.deallocate(dp, static_cast<size_t>(lenth));
        });
    std::memcpy(package.get(), data.data(), size);

    // Endianness conversion
    package->length = swapNetworkEndianness(package->length);
    package->requestID = swapNetworkEndianness(package->requestID);
    package->type = static_cast<DataPackageType>(swapNetworkEndianness(static_cast<int>(package->type)));
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
    this->type = static_cast<DataPackageType>(swapNetworkEndianness(static_cast<int>(this->type)));
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

} // namespace qls
