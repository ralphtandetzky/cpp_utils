#pragma once

#include <atomic>

class SpinLock
{

public:
    SpinLock()
        : state(Unlocked)
    {
    }

    void lock()
    {
        while ( state.exchange(Locked, std::memory_order_acquire) == Locked )
        { /* busy-wait */ }
    }

    void unlock()
    {
        state.store(Unlocked, std::memory_order_release);
    }

private:
    typedef enum { Locked, Unlocked } LockState;
    std::atomic<LockState> state;
};
