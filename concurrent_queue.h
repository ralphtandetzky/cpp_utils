/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <mutex>
#include <condition_variable>
#include <list>

#include "cpp_utils/locking.h"
#include "cpp_utils/scope_guard.h"

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
        data( [&]( Data & d )
        {
            d.q.splice( end(d.q), move(node) );
            d.cv.notify_one();
        } );
    }

    void push( const T &  t ) { emplace(           t  ); }
    void push(       T && t ) { emplace( std::move(t) ); }

    bool tryPop( T & t )
    {
        std::list<T> node;
        const auto success = data( [&]( Data & d ) -> bool
        {
            if ( d.q.empty() )
                return false;
            node.splice( end(node), d.q, begin(d.q) );
            return true;
        } );
        if ( success )
            t = std::move( node.front() );
        return success;
    }

    T pop()
    {
        std::list<T> node;
        data.withUniqueLock( [&]( Data & d, std::unique_lock<std::mutex> lock )
        {
            d.cv.wait( lock, [&d]{ return !d.q.empty(); } );
            node.splice( node.end(), d.q, d.q.begin() );
        } );
        return std::move( node.front() );
    }

private:
    struct Data
    {
        std::condition_variable_any cv;
        std::list<T> q;
    };
    Monitor<Data> data;
};

} // namespace cu
