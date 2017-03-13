// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

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
  template <typename ...WorkerData>
  struct WorkerDataImpl
  {
    template <typename ...Args>
    WorkerDataImpl( Args &&... args )
      : workerData( std::forward<Args>(args)... )
    {}

    std::tuple<WorkerData...> workerData;
  };

  template <typename WorkerData>
  struct WorkerDataImpl<WorkerData>
  {
    template <typename ...Args>
    WorkerDataImpl( Args &&... args )
      : workerData( std::forward<Args>(args)... )
    {}

    WorkerData workerData;
  };

  template <>
  struct WorkerDataImpl<>
  {
    struct {} workerData;
  };

  template <bool hasExternalQueue, typename ...WorkerData>
  struct GenericTaskQueueThreadData
      : WorkerDataImpl<WorkerData...>
  {
  private:
    using Base = WorkerDataImpl<WorkerData...>;

  public:
    using Base::Base;

    TaskQueueWithArgs<WorkerData&...> queue;
    bool done = false;
  };

  template <typename ...WorkerData>
  struct GenericTaskQueueThreadData<true,WorkerData...>
      : WorkerDataImpl<WorkerData...>
  {
    TaskQueueWithArgs<WorkerData&...> & queue;
    std::atomic<bool> & done;

    template <typename ...Args>
    explicit GenericTaskQueueThreadData(
        TaskQueueWithArgs<WorkerData&...> & queue_,
        std::atomic<bool> & done_,
        Args &&... args
        )
      : WorkerDataImpl<WorkerData...>( std::forward<Args>(args)... )
      , queue(queue_)
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
  std::thread worker;

  template <typename F,
            typename Arg>
  static void apply( F && f, Arg && workerData, cu::Rank<2> )
  {
    cu::apply( std::forward<F>(f), std::forward<Arg>(workerData) );
  }

  template <typename F,
            typename Arg>
  static void apply( F && f, Arg && workerData, cu::Rank<1> )
  {
    std::forward<F>(f)( std::forward<Arg>(workerData) );
  }

  template <typename F,
            typename Arg>
  static void apply( F && f, Arg &&, cu::Rank<0> )
  {
    std::forward<F>(f)();
  }

  void startWorker()
  {
    worker = std::thread( [this]()
    {
      while (!this->done)
      {
        apply( [&](auto&&...args)
          {
            this->queue.popAndExecute( std::forward<decltype(args)>(args)... );
          },
          this->workerData,
          cu::Rank<sizeof...(WorkerData)>{}
          );
      }
    } );
  }

public:
  /// Starts the task dispatching loop an another thread.
  ///
  /// This overload is enabled, iff @c hasExternalTaskQueue==false.
  ///
  /// @param args should be the constructor arguments of the worker data.
  /// If @c sizeof...(WorkerData)==0, then no arguments should be passed.
  /// If @c sizeof...(WorkerData)==1, then whatever constructor arguments
  /// for the one item should be passed. They will be forwarded.
  /// If @c sizeof...(WorkerData)>=2, then the number of arguments should
  /// coincide with it and each arguments will be forwarded to its respective
  /// worker data item.
  template <typename ...Args,
            bool B = hasExternalTaskQueue,
            typename = typename std::enable_if_t<
              !B && B == hasExternalTaskQueue>>
  explicit GenericTaskQueueThread(
      Args &&... args
      )
    : Base( std::forward<Args>(args)... )
  {
    startWorker();
  }

  /// Starts the task dispatching loop an another thread.
  ///
  /// This overload is enabled, iff @c hasExternalTaskQueue==true.
  ///
  /// @param args See the other overload for details.
  template <typename ...Args,
            bool B = hasExternalTaskQueue,
            typename = typename std::enable_if_t<B && B == hasExternalTaskQueue>>
  explicit GenericTaskQueueThread(
      TaskQueueWithArgs<WorkerData&...> & queue
    , std::atomic<bool> & done
    , Args &&... args
      )
    : Base( queue, done, std::forward<Args>(args)... )
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
