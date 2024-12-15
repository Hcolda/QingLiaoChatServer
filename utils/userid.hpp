#ifndef USER_IDENTIFICATION
#define USER_IDENTIFICATION

#include <unordered_map>

namespace qls
{

class UserID final
{
public:
    UserID():
        m_user_id(0ll) {}
    explicit UserID(long long user_id):
        m_user_id(user_id) {}
    UserID(const UserID& u):
        m_user_id(u.m_user_id) {}
    UserID(UserID&& u):
        m_user_id(u.m_user_id) {}
    ~UserID() = default;

    long long getOriginValue() const
    {
        return m_user_id;
    }

    UserID& operator=(const UserID& u)
    {
        if (&u == this)
            return *this;
        m_user_id = u.m_user_id;
        return *this;
    }

    UserID& operator=(UserID&& u)
    {
        if (&u == this)
            return *this;
        m_user_id = u.m_user_id;
        return *this;
    }

    UserID& operator=(long long user_id)
    {
        m_user_id = user_id;
        return *this;
    }

    friend bool operator==(const UserID& u1, const UserID& u2)
    {
        return u1.m_user_id == u2.m_user_id;
    }

    friend bool operator!=(const UserID& u1, const UserID& u2)
    {
        return u1.m_user_id != u2.m_user_id;
    }

    friend bool operator<(const UserID& u1, const UserID& u2)
    {
        return u1.m_user_id < u2.m_user_id;
    }

    operator long long() const
    {
        return m_user_id;
    }
    
private:
    long long m_user_id;
};

} // namespace qls

namespace std
{
    template<>
    struct hash<qls::UserID>{
    public:
        size_t operator()(const qls::UserID &u) const 
        {
            return hash<long long>()(u.getOriginValue());
        }
    };
    
    template<>
    struct equal_to<qls::UserID>{
    public:
        bool operator()(const qls::UserID &u1, const qls::UserID &u2) const
        {
            return u1 == u2;
        }
    };
}

#endif // !USER_IDENTIFICATION
