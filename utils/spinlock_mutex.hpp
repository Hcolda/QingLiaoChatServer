#ifndef SPINLOCK_MUTEX_HPP
#define SPINLOCK_MUTEX_HPP

#include <atomic>
#include <mutex>

namespace qls
{

class spinlock_mutex
{
private:
    std::atomic_flag flag;
public:
    spinlock_mutex() = default;
    ~spinlock_mutex() = default;

    void lock()
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
};

}

#endif // !SPINLOCK_MUTEX_HPP
