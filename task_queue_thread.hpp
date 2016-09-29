/** @file Defines the class template @c GenericTaskQueueThread.
 * @author Ralph Tandetzky
 */

#pragma once

#include "c++17_features.hpp"
#include "task_queue.hpp"

#include <thread>
#include <tuple>

namespace cu
{

namespace detail
{
  template <bool hasExternalQueue, typename ...WorkerData>
  struct GenericTaskQueueThreadData
  {
    TaskQueueWithArgs<WorkerData&...> queue;
    bool done = false;
  };

  template <typename ...WorkerData>
  struct GenericTaskQueueThreadData<true,WorkerData...>
  {
    TaskQueueWithArgs<WorkerData&...> & queue;
    std::atomic<bool> & done;

    explicit GenericTaskQueueThreadData(
        TaskQueueWithArgs<WorkerData&...> & queue_,
        std::atomic<bool> & done_ )
      : queue(queue_)
      , done(done_)
    {}
  };
} // namespace detail

/// A task dispatching thread class.
///
/// The constructor starts the dispatching loop automatically. New tasks
/// are push into the task queue using the function call operator.
/// The destructor blocks and waits until all tasks are dispatched.
///
/// This class has one @c bool template parameter, which tells whether
/// the used @c TaskQueue is external to the object.
/// The @c WorkerData template parameter pack can be used to pass
/// data that is associated with the worker thread to the tasks.
/// This can be used to optimize execution by caching data, for example.
///
/// Most of the time, the alias @c TaskQueueThread for
/// @c GenericTaskQueueThread<false> is used.
/// The @c TaskQueueThread contains its own @c TaskQueue.
///
/// If a @c TaskQueue should be dispatched by multiple threads, then
/// @c ExternalTaskQueueThread can be used, which is an alias
/// for @c GenericTaskQueueThread<true>. You may consider to use
/// @c TaskQueueThreadPool for convenience in this case.
template <bool hasExternalTaskQueue,
          typename ...WorkerData>
class GenericTaskQueueThread
    : private detail::GenericTaskQueueThreadData<hasExternalTaskQueue, WorkerData...>
{
private:
  using Base = detail::GenericTaskQueueThreadData<hasExternalTaskQueue, WorkerData...>;
  std::tuple<WorkerData...> workerData;
  std::thread worker;

  void startWorker()
  {
    worker = std::thread( [this]()
    {
      while (!this->done)
      {
        cu::apply( [&](auto&&...args)
          {
            this->queue.popAndExecute( std::forward<decltype(args)>(args)... );
          },
          workerData
          );
      }
    } );
  }

public:
  /// Starts the task dispatching loop an another thread.
  template <bool B = hasExternalTaskQueue,
            typename = typename std::enable_if<!B && B == hasExternalTaskQueue>::type>
  explicit GenericTaskQueueThread(
      WorkerData &&... data
      )
    : workerData( std::forward_as_tuple( std::forward<WorkerData>(data)... ) )
  {
    startWorker();
  }

  /// Starts the task dispatching loop an another thread.
  template <bool B = hasExternalTaskQueue,
            typename = typename std::enable_if<B && B == hasExternalTaskQueue>::type>
  explicit GenericTaskQueueThread(
      TaskQueueWithArgs<WorkerData&...> & queue
    , std::atomic<bool> & done
    , WorkerData &&... data
      )
    : Base( queue, done )
    , workerData( std::forward_as_tuple( std::forward<WorkerData>(data)... ) )
  {
    startWorker();
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
    return this->queue.push( std::forward<F>(f) );
  }

  /// Blocks until all tasks in the queue have been dispatched and the
  /// thread has ended its execution.
  ~GenericTaskQueueThread()
  {
    (*this)([this](WorkerData&...){ this->done = true; });
    worker.join();
  }
};

using TaskQueueThread         = GenericTaskQueueThread<false>;
using ExternalTaskQueueThread = GenericTaskQueueThread<true >;

} // namespace cu
