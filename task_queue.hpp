// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

/** @file Defines the class @c TaskQueueWithArgs.
 * @author Ralph Tandetzky
 */

#pragma once

#include "concurrent_queue.hpp"
#include <future>
#include <memory>
#include <type_traits>

namespace cu
{

/// A high-performance concurrent queue for functors.
///
/// This class is suitable as a task queue for an event loop of a thread
/// or for dispatching tasks in a threadpool.
///
/// Most of the time, the template argument list is empty.
/// For this case, the type alias @c TaskQueue for @c TaskQueueWithArgs<>
/// exists.
///
/// On occasion, template arguments @c Args can be used to pass a reference
/// to the data of a worker thread as additional argument to the tasks for
/// optimization purposes.
/// This technique can be used to cache data in a worker thread, for example.
template <typename ...Args>
class TaskQueueWithArgs
{
public:
  /// Puts a task into the queue.
  ///
  /// @returns a future for the result of the functor.
  template <typename F>
  auto push( F && f )
  {
#if !defined(_MSC_VER)
    auto task = std::packaged_task<std::result_of_t<F(Args...)>(Args&&...)>(
          std::forward<F>(f) );
    auto result = task.get_future();
    tasks.emplace( std::move(task) );
    return result;
#else
    // This is a work-around, since MSVC is not standard compliant.
    // MSVC does not allow move-only functors to be passed to a
    // packaged_task constructor except the move constructor.
    // Since packaged_task is move-only by itself, packaged_tasks with
    // different template arguments cannot be passed into each others
    // constructors.
    auto fPtr = std::make_shared<F>( std::forward<F>(f) );
    auto task = std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>(Args&&...)>>(
          [fPtr]( Args &&... args ){ return (*fPtr)( std::forward<Args>(args)... ); } );
    auto result = task->get_future();
    tasks.emplace(
          [task]( Args &&... args )
          {
              (*task)( std::forward<Args>(args)... );
          } );
    return result;
#endif
  }

  /// Pops the oldest element in the queue in a blocking way and executes it.
  void popAndExecute( Args &&... args )
  {
    tasks.pop()( std::forward<Args>(args)... );
  }

private:
  ConcurrentQueue<std::packaged_task<void(Args&&...)>> tasks;
};

using TaskQueue = TaskQueueWithArgs<>;

} // namespace cu
