/** @file Defines the class @c TaskQueueThread.
 * @author Ralph Tandetzky
 */

#pragma once

#include "task_queue.hpp"

#include <thread>

namespace cu
{

/// A task dispatching thread class.
///
/// The constructor starts the dispatching loop automatically. New tasks
/// are push into the task queue using the function call operator.
/// The destructor blocks and waits until all tasks are dispatched.
class TaskQueueThread
{
private:
  TaskQueue queue;
  bool done = false;
  std::thread worker;

public:
  /// Starts the task dispatching loop an another thread.
  TaskQueueThread()
  {
    worker = std::thread( [this]()
    {
      while (!done)
      {
        queue.popAndExecute();
      }
    });
  }

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

  /// Blocks until all tasks in the queue have been dispatched and the
  /// thread has ended its execution.
  ~TaskQueueThread()
  {
    (*this)([this](){ done = true; });
    worker.join();
  }
};

} // namespace cu
