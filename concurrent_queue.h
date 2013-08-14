#pragma once

#include "locking.h"
#include "scope_guard.h"
#include "spin_lock.h"
#include "std_make_unique.h"

#include <condition_variable>
#include <memory>
#include <mutex>

template <typename T>
class ConcurrentQueue
{
public:
    ConcurrentQueue()
        : lastNext(&first)
        , nWaitingThreads(0)
    {
    }

    template <typename ...Args>
    void emplace( Args&&...args )
    {
        auto p = std::make_unique<Node>( std::forward<Args>(args)... );
        std::lock_guard<SpinLock> lock( mutex );
        *lastNext = std::move(p);
        lastNext = &lastNext->get()->next;
        if ( nWaitingThreads > 0 )
            cv.notify_one();
    }

    void push( const T &  t ) { emplace(           t  ); }
    void push(       T && t ) { emplace( std::move(t) ); }

    bool tryPop( T & t )
    {
        std::unique_ptr<Node> p;
        {
            MAKE_LOCK_GUARD( mutex );
            if ( first )
            {
                p = std::move(first);
                first = std::move(p->next);
            }
            else
                return false;
        }
        t = std::move(p.data);
        return true;
    }

    T pop()
    {
        std::unique_ptr<Node> p;
        {
            auto lock = makeUniqueLock( mutex );
            ++nWaitingThreads;
            while ( !first )
                cv.wait( lock );
            --nWaitingThreads;
            p = std::move(first);
            first = std::move(p->next);
        }
        assert( p );
        return std::move(p->data);
    }

private:
    struct Node
    {
        template <typename ...Args>
        Node( Args&&...args )
            : data( std::forward<Args>(args)... )
        {
        }

        T data;
        std::unique_ptr<Node> next;
    };

    std::unique_ptr<Node> first;
    std::unique_ptr<Node> * lastNext;
    SpinLock mutex;
    std::condition_variable_any cv;
    size_t nWaitingThreads;
};
