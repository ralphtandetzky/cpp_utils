/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include "concurrent_queue.h"
#include <algorithm>
#include <future>
#include <thread>
#include <vector>

namespace cu {

/// @brief Implements a thread pool. Tasks can easily be added to a task queue.
///
/// This class is best suited for implementing parallel algorithms that use
/// independent tasks. It can also be used as a background worker thread by
/// specifying the number of threads to be @c 1 at construction. 
///
/// The number of threads is immutable during the life-time of the object. 
/// The destructor waits for all tasks to be finished and closes down all 
/// threads. 
///
/// Thread-safety: All functions in this class are thread-safe.
class ParallelExecutor
{
public:
    /// Starts the worker threads. The number of threads used is passed. If 
    /// @c 0 is passed, then the number of cpu cored will be used instead.
    explicit ParallelExecutor( size_t nThreads = 0 )
        : done(false)
    {
        if ( nThreads == 0 )
            nThreads = std::thread::hardware_concurrency();
        for ( size_t i = 0; i < nThreads; ++i )
            threads.emplace_back( [=]() { while ( !done ) q.pop()(); });
    }

    /// Waits for all tasks to finish and shuts down all the worker 
    /// threads. 
    ~ParallelExecutor()
    {
        for ( size_t i = 0; i < threads.size(); ++i )
            q.emplace( [=]() { done = true; });
        std::for_each( begin(threads), end(threads),
                       std::mem_fn(&std::thread::join) );
    }

    /// Pushes a task to the task queue and returns a future through which 
    /// the result of the calculation can later be retrieved. 
    ///
    /// @warning It is not safe to let a task wait another task that shall 
    /// be started later. This may result in dead-lock, if all tasks
    /// wait for other tasks to do something. Hence tasks should never 
    /// depend on other tasks other than tasks that have been added to the
    /// task queue before them.
    template <typename F>
    auto addTask( F && f ) -> std::future<decltype(f())>
    {
        auto task = std::make_shared<std::packaged_task<decltype(f())()> >(
            std::forward<F>(f) );
        auto result = task->get_future();
        q.emplace( [=](){ (*task)(); } );
        return result;
    }

    /// Returns the total number of worker threads. 
    size_t getNWorkers() const
    {
        return threads.size();
    }

private:
    ConcurrentQueue<std::packaged_task<void()> > q;
    std::atomic<bool> done;
    std::vector<std::thread> threads;
};

} // namespace cu
