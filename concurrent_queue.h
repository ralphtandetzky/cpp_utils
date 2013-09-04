#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

#include "locking.h"
#include "scope_guard.h"

namespace cu {

template <typename T>
class ConcurrentQueue
{
public:
    template <typename ...Args>
    void emplace( Args&&...args )
    {
        MAKE_LOCK_GUARD(m);
        q.emplace( std::forward<Args>(args)... );
        cv.notify_one();
    }

    void push( const T &  t ) { emplace(           t  ); }
    void push(       T && t ) { emplace( std::move(t) ); }

    bool tryPop( T & t )
    {
        MAKE_LOCK_GUARD(m);
        if ( q.empty() )
            return false;
        t = std::move( q.front() );
        q.pop();
        return true;
    }

    T pop()
    {
        auto lock = cu::MakeUniqueLock(m);
        cv.wait( lock, [=](){ return !q.empty(); } );
        SCOPE_SUCCESS { q.pop(); };
        return std::move( q.front() );
    }

private:
    std::mutex m;
    std::condition_variable_any cv;
    std::queue<T> q;
};

} // namespace cu
