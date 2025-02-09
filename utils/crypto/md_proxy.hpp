#ifndef MD_PROXY_HPP
#define MD_PROXY_HPP

#include <cstring>
#include <cstdio>
#include <openssl/evp.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <concepts>

#include "ossl_proxy.hpp"

namespace qls
{
    class md_proxy
    {
    public:
        md_proxy(ossl_proxy& o, std::string_view algorithm):
            m_ossl_proxy(o)
        {
            m_message_digest = EVP_MD_fetch(m_ossl_proxy.get_native(),
                                            algorithm.data(), nullptr);
            if (m_message_digest == nullptr)
                throw std::runtime_error("EVP_MD_fetch() returned NULL");
            m_digest_context = EVP_MD_CTX_new();
            if (m_digest_context == nullptr) {
                EVP_MD_free(m_message_digest);
                throw std::runtime_error("EVP_MD_CTX_new() returned NULL");
            }
            if (EVP_DigestInit(m_digest_context, m_message_digest) != 1) {
                EVP_MD_free(m_message_digest);
                EVP_MD_CTX_free(m_digest_context);
                throw std::runtime_error("EVP_DigestInit() failed");
            }
        }

        md_proxy(const md_proxy&) = delete;
        md_proxy(md_proxy&& md) noexcept:
            m_ossl_proxy(md.m_ossl_proxy)
        {
            std::swap(m_message_digest, md.m_message_digest);
            std::swap(m_digest_context, md.m_digest_context);
        }

        md_proxy& operator=(const md_proxy&) = delete;
        md_proxy& operator=(md_proxy&& md) noexcept
        {
            if (this == &md)
                return *this;

            if (m_message_digest != nullptr)
                EVP_MD_free(m_message_digest);
            if (m_digest_context != nullptr)
                EVP_MD_CTX_free(m_digest_context);
            m_message_digest = md.m_message_digest;
            m_digest_context = md.m_digest_context;
            md.m_digest_context = nullptr;
            md.m_message_digest = nullptr;
        }

        ~md_proxy() noexcept
        {
            if (m_message_digest != nullptr)
                EVP_MD_free(m_message_digest);
            if (m_digest_context != nullptr)
                EVP_MD_CTX_free(m_digest_context);
        }

        EVP_MD* get_md_native() noexcept
        {
            return m_message_digest;
        }

        EVP_MD_CTX* get_md_ctx_native() noexcept
        {
            return m_digest_context;
        }

        operator bool()
        {
            return m_message_digest && m_digest_context;
        }

        template<class... Args>
            requires requires (Args&&... args) { (std::string_view(std::forward<Args>(args)), ...); }
        std::string operator()(Args&&... args)
        {
            std::string digest_value;
            int digest_length = EVP_MD_get_size(m_message_digest);
            if (digest_length <= 0)
                throw std::runtime_error("EVP_MD_get_size() returned invalid size");

            auto input = [this](std::string_view data) {
                if (EVP_DigestUpdate(m_digest_context, data.data(), data.size()) != 1)
                    throw std::runtime_error("EVP_DigestUpdate() failed");
            };

            (input(std::forward<Args>(args)), ...);

            digest_value.resize(digest_length);
            if (EVP_DigestFinal(m_digest_context,
                                reinterpret_cast<unsigned char*>(digest_value.data()),
                                nullptr) != 1) {
                throw std::runtime_error("EVP_DigestFinal() failed");
            }
            std::string buffer;
            std::size_t buffer_size = digest_length * 2;
            buffer.resize(buffer_size + 1);
            char* buffer_pointer = buffer.data();
            for (std::size_t i = 0, j = 0; j < digest_length; i = i + 2, ++j) {
#ifdef _MSC_VER
                ::sprintf_s(buffer_pointer + i,
                            buffer_size + 1 - i,
                            "%02x",
                            reinterpret_cast<const unsigned char*>(digest_value.c_str())[j]);
#else
                std::sprintf(buffer_pointer + i,
                             "%02x",
                             reinterpret_cast<const unsigned char*>(digest_value.c_str())[j]);
#endif
            }
            buffer.resize(buffer_size);
            return buffer;
        }

    private:
        ossl_proxy& m_ossl_proxy;
        EVP_MD*     m_message_digest;
        EVP_MD_CTX* m_digest_context;
    };
} // namespace qls

#endif // MD_PROXY_HPP
