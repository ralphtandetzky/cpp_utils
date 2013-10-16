/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <mutex>
#include <condition_variable>
#include <list>

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
        std::list<T> node;
        node.emplace_back( std::forward<Args>(args)... );
        MAKE_LOCK_GUARD(m);
        q.splice( end(q), move(node) );
        cv.notify_one();
    }

    void push( const T &  t ) { emplace(           t  ); }
    void push(       T && t ) { emplace( std::move(t) ); }

    bool tryPop( T & t )
    {
        std::list<T> node;
        {
            MAKE_LOCK_GUARD(m);
            if ( q.empty() )
                return false;
            node.splice( end(node), begin(q) );
        }
        t = std::move( q.front() );
        return true;
    }

    T pop()
    {
        std::list<T> node;
        {
            auto lock = cu::MakeUniqueLock(m);
            cv.wait( lock, [=]{ return !q.empty(); } );
            node.splice( node.end(), q, q.begin() );
        }
        return std::move( node.front() );
    }

private:
    std::mutex m;
    std::condition_variable_any cv;
    std::list<T> q;
};

} // namespace cu
