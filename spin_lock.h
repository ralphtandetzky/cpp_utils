#pragma once

#include <atomic>

namespace cu {

class SpinLock
{

public:
    SpinLock()
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
    typedef enum { locked, unlocked } LockState;
    std::atomic<LockState> state;
};

} // namespace cu
