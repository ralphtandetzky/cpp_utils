/** @file Defines the class @c TaskQueueThreadPool.
 * @author Ralph Tandetzky
 */

#pragma once

#include "functors.hpp"
#include "task_queue_thread.hpp"

#include <vector>

namespace cu
{

/// A task dispatching thread pool class.
///
/// This is like @c TaskQueueThread, but the tasks may be dispatched by
/// multiple threads concurrently.
///
/// @note The @c WorkerData will be assigned to each thread in the thread pool
/// making the data thread-local in effect (as long as the types aren't
/// references).
template <typename ...WorkerData>
class TaskQueueThreadPool
{
private:
  TaskQueueWithArgs<WorkerData&...> queue;
  std::atomic<bool> done{false};
  std::vector<
    std::unique_ptr<
      GenericTaskQueueThread<
        true,
        WorkerData...>>> workers;

  auto computeNWorkers( std::size_t nThreads )
  {
    if ( nThreads != 0 )
      return nThreads;

    return std::max( std::size_t{1}, std::thread::hardware_concurrency() );
  }

public:
  ~TaskQueueThreadPool()
  {
    done = true;
    for ( const auto & worker : workers )
      (void)worker, (*this)( NoOpFunctor{} );
  }

  /// Starts the task dispatching loops of the thread pool.
  explicit TaskQueueThreadPool(
      std::size_t nThreads,
      WorkerData &&... workerData )
  {
    nThreads = computeNWorkers( nThreads );
    workers.reserve( nThreads );
    for ( auto i = 0*nThreads; i < nThreads; ++i )
      workers.push_back(
            std::make_unique<GenericTaskQueueThread<true,WorkerData...>>(
              queue, done, std::forward<WorkerData>(workerData)...) );
  }

  explicit TaskQueueThreadPool(
      WorkerData &&... workerData )
    : TaskQueueThreadPool( 0, std::forward<WorkerData>(workerData)... )
  {}

  /// Adds a task to the event queue.
  ///
  /// @returns A @c std::future for the result.
  ///
  /// @example A task is added like this:
  ///   @code
  ///     auto result = taskQueueThread( [](){ return doSomething(); } );
  ///   @endcode
  template <typename F>
  auto operator()( F && f )
  {
    return queue.push( std::forward<F>(f) );
  }
};

} // namespace cu
