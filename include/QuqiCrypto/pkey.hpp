#ifndef QUQICRYPTO_PKEY_HPP
#define QUQICRYPTO_PKEY_HPP

#include <string>
#include <mutex>
#include <memory>

#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include "md.hpp"
#include "definition.hpp"

QUQICRYPTO_NAMESPACE_BEGIN

class PEM;

class pkey
{
public:
    pkey() = default;
    ~pkey() = default;

    class PublicKey;
    class KeyGenerator;

    class PrivateKey
    {
    public:
        PrivateKey() : shared_pkey_(nullptr) {}
        PrivateKey(const PrivateKey& p)
        {
            shared_pkey_ = p.shared_pkey_;
        }
        PrivateKey(PrivateKey&& p) noexcept
        {
            shared_pkey_ = std::move(p.shared_pkey_);
        }
        ~PrivateKey() = default;

        operator bool() const
        {
            return shared_pkey_.get() != nullptr;
        }

        PrivateKey& operator=(const PrivateKey& p)
        {
            if (this == &p)
                return *this;

            shared_pkey_ = p.shared_pkey_;

            return *this;
        }

        PrivateKey& operator=(PrivateKey&& p) noexcept
        {
            if (this == &p)
                return *this;

            shared_pkey_ = std::move(p.shared_pkey_);

            return *this;
        }

        friend class pkey;
        friend class PEM;

    protected:
        PrivateKey(EVP_PKEY* pkey)
        {
            if (!pkey)
                throw std::logic_error("invalid");
            shared_pkey_ = std::shared_ptr<EVP_PKEY>(pkey, [](EVP_PKEY* pkey) {EVP_PKEY_free(pkey); });
        }

        std::shared_ptr<EVP_PKEY> shared_pkey_;
    };

    class PublicKey
    {
    public:
        PublicKey() : shared_pkey_(nullptr) {}
        PublicKey(const PublicKey& p)
        {
            shared_pkey_ = p.shared_pkey_;
        }
        PublicKey(PublicKey&& p) noexcept
        {
            shared_pkey_ = std::move(p.shared_pkey_);
        }
        PublicKey(const PrivateKey& p)
        {
            shared_pkey_ = p.shared_pkey_;
        }
        PublicKey(PrivateKey&& p) noexcept
        {
            shared_pkey_ = std::move(p.shared_pkey_);
        }
        ~PublicKey() = default;

        operator bool() const
        {
            return shared_pkey_.get() != nullptr;
        }

        PublicKey& operator=(const PublicKey& p)
        {
            if (this == &p)
                return *this;

            shared_pkey_ = p.shared_pkey_;

            return *this;
        }

        PublicKey& operator=(PublicKey&& p) noexcept
        {
            if (this == &p)
                return *this;

            shared_pkey_ = std::move(p.shared_pkey_);

            return *this;
        }

        friend class PEM;
        friend class pkey;

    protected:
        PublicKey(EVP_PKEY* pkey)
        {
            if (!pkey)
                throw std::logic_error("invalid");
            shared_pkey_ = std::shared_ptr<EVP_PKEY>(pkey, [](EVP_PKEY* pkey) {EVP_PKEY_free(pkey); });
        }

        std::shared_ptr<EVP_PKEY> shared_pkey_;
    };

    class KeyGenerator
    {
    public:
        KeyGenerator() = default;
        ~KeyGenerator() = default;

        static PrivateKey generateRSA(int bits)
        {
            return { EVP_RSA_gen(bits) };
        }

        [[deprecated("it can't be used now")]] static PrivateKey generateECC(const std::string& curveName)
        {
            std::shared_ptr<EVP_PKEY> lp(EVP_EC_gen(curveName.c_str()), [](EVP_PKEY* pkey) {EVP_PKEY_free(pkey); });
            EVP_PKEY* lpkey = EVP_PKEY_new();
            std::string pemData;
            {
                std::shared_ptr<BIO> shared_bio(BIO_new(BIO_s_mem()), [](BIO* bio) {BIO_vfree(bio); });

                //i2d_PUBKEY_bio(shared_bio.get(), lp.get());
                i2d_PrivateKey_bio(shared_bio.get(), lp.get());

                constexpr int bufferSize = 512;
                size_t size = bufferSize;
                int code = 0;
                size_t j = 0;
                for (int i = 0; ; i++)
                {
                    pemData.resize(size);
                    code = BIO_read(shared_bio.get(), (char*)pemData.data() + j, bufferSize);
                    if (code < bufferSize)
                        break;
                    size += bufferSize;
                    j += bufferSize;
                }
                pemData.resize(j + code);
            }
            {
                std::shared_ptr<BIO> shared_bio(BIO_new(BIO_s_mem()), [](BIO* bio) {BIO_vfree(bio); });

                BIO_write(shared_bio.get(), pemData.c_str(), int(pemData.size()));
                d2i_PrivateKey_bio(shared_bio.get(), &lpkey);
                //shared_pkey_ = std::shared_ptr<EVP_PKEY>(lpkey, [](EVP_PKEY* pkey) {EVP_PKEY_free(pkey); });
            }

            return { lpkey };
        }
    };

    static bool encrypt(const std::string& in, std::string& out, const PublicKey& pubKey)
    {
        if (in.empty() || !pubKey)
            return false;

        std::shared_ptr<EVP_PKEY_CTX> shared_ctx(EVP_PKEY_CTX_new(pubKey.shared_pkey_.get(), nullptr),
            [](EVP_PKEY_CTX* ctx) {EVP_PKEY_CTX_free(ctx); });
        if (EVP_PKEY_encrypt_init(shared_ctx.get()) <= 0)
            return false;

        size_t outl = 8192;
        if (EVP_PKEY_encrypt(shared_ctx.get(), nullptr, &outl, (const unsigned char*)in.c_str(), in.size()) <= 0)
        {
            out.resize(0);
            return false;
        }

        out.resize(outl);
        int code = EVP_PKEY_encrypt(shared_ctx.get(), (unsigned char*)out.data(),
            &outl, (const unsigned char*)in.c_str(), in.size());
        if (code <= 0)
        {
            return false;
        }

        out.resize(outl);

        return true;
    }

    static bool decrypt(const std::string& in, std::string& out, const PrivateKey& priKey)
    {
        if (in.empty() || !priKey)
            return false;

        std::shared_ptr<EVP_PKEY_CTX> shared_ctx(EVP_PKEY_CTX_new(priKey.shared_pkey_.get(), nullptr),
            [](EVP_PKEY_CTX* ctx) {EVP_PKEY_CTX_free(ctx); });
        if (EVP_PKEY_decrypt_init(shared_ctx.get()) <= 0)
            return false;

        size_t outl = 8192;
        if (EVP_PKEY_decrypt(shared_ctx.get(), nullptr, &outl, (const unsigned char*)in.c_str(), in.size()) <= 0)
        {
            out.resize(0);
            return false;
        }

        out.resize(outl);
        if (EVP_PKEY_decrypt(shared_ctx.get(), (unsigned char*)out.data(),
            &outl, (const unsigned char*)in.c_str(), in.size()) <= 0)
        {
            return false;
        }

        out.resize(outl);

        return true;
    }

    static bool signature(const std::string& message, std::string& out, const PrivateKey& prikey, MDMode mode)
    {
        if (mode == MDMode::null || message.empty() || !prikey)
            return false;

        std::shared_ptr<EVP_MD_CTX> shared_ctx(EVP_MD_CTX_new(), [](EVP_MD_CTX* ctx) {EVP_MD_CTX_free(ctx); });
        if (EVP_DigestSignInit(shared_ctx.get(), nullptr, MD(mode), nullptr, prikey.shared_pkey_.get()) <= 0)
            return false;

        if (EVP_DigestSignUpdate(shared_ctx.get(), message.c_str(), message.size()) <= 0)
            return false;

        size_t size = 0;
        if (EVP_DigestSignFinal(shared_ctx.get(), nullptr, &size) <= 0)
            return false;
        out.resize(size);

        if (EVP_DigestSignFinal(shared_ctx.get(), (unsigned char*)out.data(), &size) <= 0)
            return false;
        return true;
    }

    static bool verify(const std::string& message, const std::string& signature, const PublicKey& pubkey, MDMode mode)
    {
        if (mode == MDMode::null)
            throw std::logic_error("mode is invalid");
        else if (message.empty())
            throw std::logic_error("message is empty");
        else if (!pubkey)
            throw std::logic_error("public key is empty");

        std::shared_ptr<EVP_MD_CTX> shared_ctx(EVP_MD_CTX_new(), [](EVP_MD_CTX* ctx) {EVP_MD_CTX_free(ctx); });
        if (EVP_DigestVerifyInit(shared_ctx.get(), nullptr, MD(mode), nullptr, pubkey.shared_pkey_.get()) <= 0)
            throw std::runtime_error("EVP_DigestVerifyInit");

        if (EVP_DigestVerifyUpdate(shared_ctx.get(), message.c_str(), message.size()) <= 0)
            throw std::runtime_error("EVP_DigestVerifyInit");

        if (!EVP_DigestVerifyFinal(shared_ctx.get(), (const unsigned char*)signature.c_str(), signature.size()))
            return false;
        return true;
    }
};

class PEM
{
public:
    PEM() = default;
    ~PEM() = default;

    static bool PEMReadPublicKey(const std::string pemData, pkey::PublicKey& key)
    {
        std::shared_ptr<BIO> shared_bio(BIO_new(BIO_s_mem()), [](BIO* bio) {BIO_vfree(bio); });

        BIO_write(shared_bio.get(), (const char*)pemData.c_str(), int(pemData.size()));


        auto pointer = EVP_PKEY_new();
        if (!PEM_read_bio_PUBKEY(shared_bio.get(), &pointer, nullptr, nullptr))
        {
            EVP_PKEY_free(pointer);
            return false;
        }
        key.shared_pkey_ = std::shared_ptr<EVP_PKEY>(pointer, [](EVP_PKEY* pkey) {EVP_PKEY_free(pkey); });

        return true;
    }

    static bool PEMReadPrivateKey(const std::string pemData, pkey::PrivateKey& key)
    {
        std::shared_ptr<BIO> shared_bio(BIO_new(BIO_s_mem()), [](BIO* bio) {BIO_vfree(bio); });

        BIO_write(shared_bio.get(), (const char*)pemData.c_str(), int(pemData.size()));

        auto pointer = key.shared_pkey_.get();
        if (!PEM_read_bio_PrivateKey(shared_bio.get(), &pointer, nullptr, nullptr))
        {
            EVP_PKEY_free(pointer);
            return false;
        }
        key.shared_pkey_ = std::shared_ptr<EVP_PKEY>(pointer, [](EVP_PKEY* pkey) {EVP_PKEY_free(pkey); });

        return true;
    }

    static bool PEMWritePublicKey(const pkey::PublicKey& key, std::string& pemData)
    {
        std::shared_ptr<BIO> shared_bio(BIO_new(BIO_s_mem()), [](BIO* bio) {BIO_vfree(bio); });

        if (PEM_write_bio_PUBKEY(shared_bio.get(), key.shared_pkey_.get()) == -1)
        {
            return false;
        }

        constexpr int bufferSize = 512;
        size_t size = bufferSize;
        int code = 0;
        size_t j = 0;
        for (int i = 0; ; i++)
        {
            pemData.resize(size);
            code = BIO_read(shared_bio.get(), (char*)pemData.data() + j, bufferSize);
            if (code == -1)
            {
                return false;
            }
            else if (code < bufferSize)
                break;
            size += bufferSize;
            j += bufferSize;
        }
        pemData.resize(j + code);

        return true;
    }

    static bool PEMWritePrivateKey(const pkey::PrivateKey& key, std::string& pemData)
    {
        std::shared_ptr<BIO> shared_bio(BIO_new(BIO_s_mem()), [](BIO* bio) {BIO_vfree(bio); });

        if (PEM_write_bio_PrivateKey(shared_bio.get(), key.shared_pkey_.get(), nullptr, nullptr, 0, nullptr, nullptr) == -1)
        {
            return false;
        }

        constexpr int bufferSize = 512;
        size_t size = bufferSize;
        int code = 0;
        size_t j = 0;
        for (int i = 0; ; i++)
        {
            pemData.resize(size);
            code = BIO_read(shared_bio.get(), (char*)pemData.data() + j, bufferSize);
            if (code == -1)
            {
                return false;
            }
            else if (code < bufferSize)
                break;
            size += bufferSize;
            j += bufferSize;
        }
        pemData.resize(j + code);

        return true;
    }
};

QUQICRYPTO_NAMESPACE_END

#endif // !QUQICRYPTO_PKEY_HPP
