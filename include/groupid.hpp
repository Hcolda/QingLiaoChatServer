#ifndef GROUP_IDENTIFICATION
#define GROUP_IDENTIFICATION

#include <unordered_map>

namespace qls
{

class GroupID final
{
public:
    GroupID():
        m_group_id(0ll) {}
    explicit GroupID(long long group_id):
        m_group_id(group_id) {}
    GroupID(const GroupID& u):
        m_group_id(u.m_group_id) {}
    GroupID(GroupID&& u):
        m_group_id(u.m_group_id) {}
    ~GroupID() = default;

    long long getOriginValue() const
    {
        return m_group_id;
    }

    GroupID& operator=(const GroupID& u)
    {
        if (&u == this) return *this;
        m_group_id = u.m_group_id;
        return *this;
    }

    GroupID& operator=(GroupID&& u)
    {
        if (&u == this) return *this;
        m_group_id = u.m_group_id;
        return *this;
    }

    GroupID& operator=(long long group_id)
    {
        m_group_id = group_id;
        return *this;
    }

    friend bool operator==(const GroupID& g1, const GroupID& g2)
    {
        return g1.m_group_id == g1.m_group_id;
    }

    friend bool operator!=(const GroupID& g1, const GroupID& g2)
    {
        return g1.m_group_id != g1.m_group_id;
    }

    friend bool operator<(const GroupID& g1, const GroupID& g2)
    {
        return g1.m_group_id < g1.m_group_id;
    }

    operator long long()
    {
        return m_group_id;
    }

private:
    long long m_group_id;
};

} // namespace qls

namespace std
{
    template<>
    struct hash<qls::GroupID>{
    public:
        size_t operator()(const qls::GroupID &g) const 
        {
            return hash<long long>()(g.getOriginValue());
        }
    };
    
    template<>
    struct equal_to<qls::GroupID>{
    public:
        bool operator()(const qls::GroupID &g1, const qls::GroupID &g2) const
        {
            return g1 == g2;
        }
    };
}

#endif // !GROUP_IDENTIFICATION
