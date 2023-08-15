#ifndef QUQICRYPTO_MD_HPP
#define QUQICRYPTO_MD_HPP

#include <memory>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/md5.h>

#include "definition.hpp"

QUQICRYPTO_NAMESPACE_BEGIN

enum class MDMode
{
    null,
    sha1,
    sha256,
    sha512,
    md5
};

class MD
{
public:
    MD() : mode_(MDMode::null) {}
    MD(MDMode mode) :
        mode_(mode)
    {
        if (mode == MDMode::null)
            throw std::logic_error("mode is invalid");
    }
    MD(const MD& m) : mode_(m.mode_) {}
    MD(MD&& m) noexcept : mode_(m.mode_) {}

    operator const EVP_MD* ()
    {
        switch (mode_)
        {
        case qcrypto::MDMode::sha1:
            return EVP_sha1();
        case qcrypto::MDMode::sha256:
            return EVP_sha256();
        case qcrypto::MDMode::sha512:
            return EVP_sha512();
        case qcrypto::MDMode::md5:
            return EVP_md5();
        default:
            throw std::logic_error("mode is invalid");
            break;
        }
    }

    MD& operator=(const MD& m)
    {
        if (this == &m)
            return *this;

        mode_ = m.mode_;
        return *this;
    }

    MD& operator=(MD&& m) noexcept
    {
        if (this == &m)
            return *this;

        mode_ = m.mode_;
        return *this;
    }

private:
    MDMode mode_;
};

QUQICRYPTO_NAMESPACE_END

#endif // !QUQICRYPTO_MD_HPP
