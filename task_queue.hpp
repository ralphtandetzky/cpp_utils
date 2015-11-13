/** @file Defines the class @c TaskQueue.
 * @author Ralph Tandetzky
 */

#pragma once

#include "concurrent_queue.hpp"
#include <future>
#include <type_traits>

namespace cu
{

/// A high-performance concurrent queue for functors.
///
/// This class is suitable as a task queue for an event loop of a thread
/// or for dispatching tasks in a threadpool.
class TaskQueue
{
public:
  /// Puts a task into the queue.
  ///
  /// @returns a future for the result of the functor.
  template <typename F>
  auto push( F && f )
  {
    auto task = std::packaged_task<std::result_of_t<F()>()>(
          std::forward<F>(f) );
    auto result = task.get_future();
    tasks.emplace( std::move(task) );
    return result;
  }

  /// Pops the oldest element in the queue in a blocking way and executes it.
  void popAndExecute()
  {
    tasks.pop()();
  }

private:
  ConcurrentQueue<std::packaged_task<void()>> tasks;
};

} // namespace cu
