#pragma once

#include <atomic>

namespace cu {

class spin_lock
{

public:
    spin_lock()
        : state(unlocked)
    {
    }

    void lock()
    {
        while ( state.exchange(locked, std::memory_order_acquire) == locked )
        { /* busy-wait */ }
    }

    void unlock()
    {
        state.store(unlocked, std::memory_order_release);
    }

private:
    typedef enum { locked, unlocked } lock_state;
    std::atomic<lock_state> state;
};

} // namespace cu
