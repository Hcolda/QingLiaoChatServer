#ifndef QUQICRYPTO_BASE64_HPP
#define QUQICRYPTO_BASE64_HPP

#include <string>
#include <memory>

#include <openssl/evp.h>

#include "definition.hpp"

QUQICRYPTO_NAMESPACE_BEGIN

class Base64
{
public:
    Base64() = default;

    ~Base64() = default;

    static bool encrypt(const std::string& in, std::string& out, bool enc)
    {
        if (enc)
        {
            std::shared_ptr<EVP_ENCODE_CTX> shared_ctx(EVP_ENCODE_CTX_new(), [](EVP_ENCODE_CTX* ctx) {EVP_ENCODE_CTX_free(ctx); });

            EVP_EncodeInit(shared_ctx.get());

            // 进行加密操作
            int mlen = 0;
            out.resize(in.size() / 3 * 4 + 8);
            int ret = EVP_EncodeUpdate(shared_ctx.get(), (unsigned char*)out.data(), &mlen, (const unsigned char*)in.data(), int(in.size()));
            if (ret != 1)
            {
                return false;
            }

            // 结束加密操作
            int flen = 0;
            EVP_EncodeFinal(shared_ctx.get(), (unsigned char*)out.data() + mlen, &flen);
            out.resize(size_t(mlen) + size_t(flen));
            return true;
        }
        else
        {
            std::shared_ptr<EVP_ENCODE_CTX> shared_ctx(EVP_ENCODE_CTX_new(), [](EVP_ENCODE_CTX* ctx) {EVP_ENCODE_CTX_free(ctx); });

            EVP_DecodeInit(shared_ctx.get());

            // 进行解密操作
            int mlen = 0;
            out.resize(in.size());
            int ret = EVP_DecodeUpdate(shared_ctx.get(), (unsigned char*)out.data(), &mlen, (const unsigned char*)in.data(), int(in.size()));
            /*if (ret != 1)
            {
                return false;
            }*/

            // 结束解密操作
            int flen = 0;
            ret = EVP_DecodeFinal(shared_ctx.get(), (unsigned char*)out.data() + mlen, &flen);
            if (ret != 1)
            {
                return false;
            }
            out.resize(size_t(mlen) + size_t(flen));
            return true;
        }
    }
};

QUQICRYPTO_NAMESPACE_END

#endif // !QUQICRYPTO_BASE64_HPP