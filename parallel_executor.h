#pragma once

#include "concurrent_queue.h"
#include <algorithm>
#include <future>
#include <thread>
#include <vector>

class ParallelExecutor
{
public:
    explicit ParallelExecutor( size_t nThreads = 0 )
        : done(false)
    {
        if ( nThreads == 0 )
            nThreads = std::thread::hardware_concurrency();
        for ( size_t i = 0; i < nThreads; ++i )
            threads.emplace_back( [=]() { while ( !done ) q.pop()(); });
    }

    ~ParallelExecutor()
    {
        for ( size_t i = 0; i < threads.size(); ++i )
            q.emplace( [=]() { done = true; });
        std::for_each( begin(threads), end(threads),
                       std::mem_fn(&std::thread::join) );
    }

    template <typename F>
    auto addTask( F && f ) -> std::future<decltype(f())>
    {
        auto task = std::make_shared<std::packaged_task<decltype(f())()>>(
            std::forward<F>(f) );
        auto result = task->get_future();
        q.emplace( [=](){ (*task)(); } );
        return result;
    }

private:
    ConcurrentQueue<std::packaged_task<void()>> q;
    std::atomic<bool> done;
    std::vector<std::thread> threads;
};
