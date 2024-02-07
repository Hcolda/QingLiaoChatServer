#include "package.h"

#include <stdexcept>

#include "networkEndinass.hpp"

void qls::Package::write(std::string_view data)
{
    m_buffer += data;
}

bool qls::Package::canRead() const
{
    using namespace qls;

    if (m_buffer.size() < sizeof(int))
        return false;

    int length = 0;
    std::memcpy(&length, m_buffer.c_str(), sizeof(int));
    length = swapNetworkEndianness(length);
    if (length > m_buffer.size())
        return false;

    return true;
}

size_t qls::Package::firstMsgLength() const
{
    using namespace qls;

    if (m_buffer.size() < sizeof(int))
        return 0;

    int length = 0;
    std::memcpy(&length, m_buffer.c_str(), sizeof(int));
    length = swapNetworkEndianness(length);
    return size_t(length);
}

std::string qls::Package::read()
{
    if (!canRead())
        throw std::logic_error("Can't read data");
    else if (!firstMsgLength())
        throw std::logic_error("length is empty");

    std::string result = m_buffer.substr(0, firstMsgLength());
    m_buffer = m_buffer.substr(firstMsgLength());

    return result;
}

const std::string& qls::Package::readBuffer() const
{
    return m_buffer;
}

void qls::Package::setBuffer(const std::string& b)
{
    m_buffer = b;
}

std::string qls::Package::makePackage(std::string_view data)
{
    int lenght = static_cast<int>(data.size());
    std::string result;
    result.resize(sizeof(int));
    std::memcpy(result.data(), result.data(), sizeof(int));
    result += data;

    return result;
}
