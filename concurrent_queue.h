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

/// @brief This class implements a concurrent queue that is especially suitable as 
/// event or task queue.
///
/// All member functions of this class are atomic except constructors and destructors 
/// which must be synchronized properly.
template <typename T>
class ConcurrentQueue
{
public:
    /// Constructs an element of type T with the given constructor arguments and puts 
    /// it at the end of the queue.
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

    /// Pushes a copy of the argument to the end of the queue.
    void push( const T &  t ) { emplace(           t  ); }
    /// Moves an element to the end of the queue.
    void push(       T && t ) { emplace( std::move(t) ); }

    /// Tries to remove an element from the queue. If the queue is not empty, then
    /// the element will be moved to the passed reference and @c true will be returned.
    /// If the queue is empty then @c false is returned in order to indicate failure.
    /// Even though this function can indicate an empty queue through its return value, 
    /// this function may throw, if other errors occur. 
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

    /// Returns the first element in the queue. If the queue is empty, this function 
    /// will block until an element is pushed to the queue. 
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
