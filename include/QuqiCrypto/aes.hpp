#ifndef QUQICRYPTO_AES_HPP
#define QUQICRYPTO_AES_HPP

#include <string>
#include <random>
#include <mutex>

#include <openssl/evp.h>

#include "definition.hpp"

QUQICRYPTO_NAMESPACE_BEGIN

class RandomGeneration
{
public:
    RandomGeneration() :
        mt_(std::random_device{}())
    {}

    ~RandomGeneration() = default;

    void generate(void* buffer, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            ((char*)buffer)[i] = char(mt_());
        }
    }
    
private:
    std::mt19937 mt_;
};

enum class AESMode
{
    ECB_128 = 0,
    ECB_192,
    ECB_256,

    CBC_128,
    CBC_192,
    CBC_256,

    CFB1_128,
    CFB1_192,
    CFB1_256,

    CFB8_128,
    CFB8_192,
    CFB8_256,

    CFB128_128,
    CFB128_192,
    CFB128_256,

    CTR_128,
    CTR_192,
    CTR_256,

    GCM_128,
    GCM_192,
    GCM_256,

    XTS_128,
    XTS_256,

    OCB_128,
    OCB_192,
    OCB_256,
};

constexpr static int AESModeLimit[26] = {
    16, 24, 32,
    16, 24, 32,
    16, 24, 32,
    16, 24, 32,
    16, 24, 32,
    16, 24, 32,
    16, 24, 32,
    16, 32,
    16, 24, 32,
};

class AES_impl
{
public:
    AES_impl()
    {
        ctx_ = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(ctx_);
    }

    ~AES_impl()
    {
        EVP_CIPHER_CTX_cleanup(ctx_);
        EVP_CIPHER_CTX_free(ctx_);
    }

protected:
    bool _encrypt(std::string_view in, std::string& out, std::string_view key, std::string_view ivec, const EVP_CIPHER* ciper, bool enc)
    {
        std::lock_guard<std::mutex> lock(ctx_mutex_);

        if (enc)
        {
            // 指定加密算法及key和iv
            int ret = EVP_EncryptInit_ex(ctx_, ciper, NULL, (const unsigned char*)key.data(), (const unsigned char*)ivec.data());
            if (ret != 1)
            {
                return false;
            }

            // 进行加密操作
            int mlen = 0;
            out.resize(in.size() + 16);
            ret = EVP_EncryptUpdate(ctx_, (unsigned char*)out.data(), &mlen, (const unsigned char*)in.data(), int(in.size()));
            if (ret != 1)
            {
                return false;
            }

            // 结束加密操作
            int flen = 0;
            ret = EVP_EncryptFinal_ex(ctx_, (unsigned char*)out.data() + mlen, &flen);
            if (ret != 1)
            {
                return false;
            }
            out.resize(size_t(mlen) + size_t(flen));
            return true;
        }
        else
        {
            // 指定解密算法及key和iv
            int ret = EVP_DecryptInit_ex(ctx_, ciper, NULL, (const unsigned char*)key.data(), (const unsigned char*)ivec.data());
            if (ret != 1)
            {
                return false;
            }

            // 进行解密操作
            int mlen = 0;
            out.resize(in.size());
            ret = EVP_DecryptUpdate(ctx_, (unsigned char*)out.data(), &mlen, (const unsigned char*)in.data(), int(in.size()));
            if (ret != 1)
            {
                return false;
            }

            // 结束解密操作
            int flen = 0;
            ret = EVP_DecryptFinal_ex(ctx_, (unsigned char*)out.data() + mlen, &flen);
            if (ret != 1)
            {
                return false;
            }
            out.resize(size_t(mlen) + size_t(flen));
            return true;
        }
    }

private:
    EVP_CIPHER_CTX* ctx_;
    std::mutex      ctx_mutex_;
};

template<AESMode mode>
class AES : public AES_impl
{
public:
    AES() = default;
    ~AES() = default;

    bool encrypt(std::string_view in, std::string& out, std::string_view key, std::string_view ivec, bool enc)
    {
        // 根据key大小创建EVP_CIPHER
        const EVP_CIPHER* cipher = nullptr;

        if (AESModeLimit[(int)mode] != key.size())
            return false;

        // 执行加解密
        switch (mode)
        {
        case qcrypto::AESMode::CBC_128:
            cipher = EVP_aes_128_cbc();
            break;
        case qcrypto::AESMode::CBC_192:
            cipher = EVP_aes_192_cbc();
            break;
        case qcrypto::AESMode::CBC_256:
            cipher = EVP_aes_256_cbc();
            break;
        case qcrypto::AESMode::CFB1_128:
            cipher = EVP_aes_128_cfb1();
            break;
        case qcrypto::AESMode::CFB1_192:
            cipher = EVP_aes_192_cfb1();
            break;
        case qcrypto::AESMode::CFB1_256:
            cipher = EVP_aes_256_cfb1();
            break;
        case qcrypto::AESMode::CFB8_128:
            cipher = EVP_aes_128_cfb8();
            break;
        case qcrypto::AESMode::CFB8_192:
            cipher = EVP_aes_192_cfb8();
            break;
        case qcrypto::AESMode::CFB8_256:
            cipher = EVP_aes_256_cfb8();
            break;
        case qcrypto::AESMode::CFB128_128:
            cipher = EVP_aes_128_cfb128();
            break;
        case qcrypto::AESMode::CFB128_192:
            cipher = EVP_aes_192_cfb128();
            break;
        case qcrypto::AESMode::CFB128_256:
            cipher = EVP_aes_256_cfb128();
            break;
        case qcrypto::AESMode::CTR_128:
            cipher = EVP_aes_128_ctr();
            break;
        case qcrypto::AESMode::CTR_192:
            cipher = EVP_aes_192_ctr();
            break;
        case qcrypto::AESMode::CTR_256:
            cipher = EVP_aes_256_ctr();
            break;
        case qcrypto::AESMode::GCM_128:
            cipher = EVP_aes_128_gcm();
            break;
        case qcrypto::AESMode::GCM_192:
            cipher = EVP_aes_192_gcm();
            break;
        case qcrypto::AESMode::GCM_256:
            cipher = EVP_aes_256_gcm();
            break;
        case qcrypto::AESMode::XTS_128:
            cipher = EVP_aes_128_xts();
            break;
        case qcrypto::AESMode::XTS_256:
            cipher = EVP_aes_256_xts();
            break;
        case qcrypto::AESMode::OCB_128:
            cipher = EVP_aes_128_ocb();
            break;
        case qcrypto::AESMode::OCB_192:
            cipher = EVP_aes_192_ocb();
            break;
        case qcrypto::AESMode::OCB_256:
            cipher = EVP_aes_256_ocb();
            break;
        default:
            return false;
        }

        return _encrypt(in, out, key, ivec, cipher, enc);
    }
};

template<>
class AES<AESMode::ECB_128> : public AES_impl
{
public:
    AES() = default;
    ~AES() = default;

    bool encrypt(std::string_view in, std::string& out, std::string_view key, bool enc)
    {
        if (AESModeLimit[(int)AESMode::ECB_128] != key.size())
            return false;

        return _encrypt(in, out, key, std::string(), EVP_aes_128_ecb(), enc);
    }
};

template<>
class AES<AESMode::ECB_192> : public AES_impl
{
public:
    AES() = default;
    ~AES() = default;

    bool encrypt(std::string_view in, std::string& out, std::string_view key, bool enc)
    {
        if (AESModeLimit[(int)AESMode::ECB_192] != key.size())
            return false;

        return _encrypt(in, out, key, std::string(), EVP_aes_192_ecb(), enc);
    }
};

template<>
class AES<AESMode::ECB_256> : public AES_impl
{
public:
    AES() = default;
    ~AES() = default;

    bool encrypt(std::string_view in, std::string& out, std::string_view key, bool enc)
    {
        if (AESModeLimit[(int)AESMode::ECB_256] != key.size())
            return false;

        return _encrypt(in, out, key, std::string(), EVP_aes_256_ecb(), enc);
    }
};

QUQICRYPTO_NAMESPACE_END

#endif // !QUQICRYPTO_AES_HPP
