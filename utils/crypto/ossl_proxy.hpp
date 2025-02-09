#ifndef OSSL_PROXY_HPP
#define OSSL_PROXY_HPP

#include <openssl/evp.h>
#include <stdexcept>

namespace qls
{
    class ossl_proxy
    {
    public:
        ossl_proxy()
        {
            m_library_context = OSSL_LIB_CTX_new();
            if (m_library_context == nullptr)
                throw std::runtime_error("OSSL_LIB_CTX_new() returned NULL");
        }

        ossl_proxy(const ossl_proxy&) = delete;
        ossl_proxy(ossl_proxy&& o) noexcept
        {
            m_library_context = o.m_library_context;
            o.m_library_context = nullptr;
        }

        ossl_proxy& operator=(const ossl_proxy&) = delete;
        ossl_proxy& operator=(ossl_proxy&& o) noexcept
        {
            if (this == &o)
                return *this;
            if (m_library_context != nullptr)
                OSSL_LIB_CTX_free(m_library_context);
            m_library_context = o.m_library_context;
            o.m_library_context = nullptr;
        }

        ~ossl_proxy() noexcept
        {
            if (m_library_context != nullptr)
                OSSL_LIB_CTX_free(m_library_context);
        }

        OSSL_LIB_CTX* get_native()
        {
            return m_library_context;
        }

    private:
        OSSL_LIB_CTX* m_library_context;
    };
} // namespace qls


#endif // OSSL_PROXY_HPP
