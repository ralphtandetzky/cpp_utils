/** @file Defines the class @c TaskQueueThread.
 * @author Ralph Tandetzky
 */

#pragma once

#include "task_queue.hpp"

#include <thread>

namespace cu
{

namespace detail
{
  template <bool hasExternalQueue>
  struct GenericTaskQueueThreadData;

  template <>
  struct GenericTaskQueueThreadData<false>
  {
    TaskQueue queue;
    bool done = false;
  };

  template <>
  struct GenericTaskQueueThreadData<true>
  {
    TaskQueue & queue;
    std::atomic<bool> done{ false };

    explicit GenericTaskQueueThreadData( TaskQueue & queue_ )
      : queue(queue_)
    {}
    GenericTaskQueueThreadData( const GenericTaskQueueThreadData & other ) = delete;
    GenericTaskQueueThreadData( GenericTaskQueueThreadData && other )
      : queue( other.queue )
      , done( other.done.load() )
    {}
  };
} // namespace detail

/// A task dispatching thread class.
///
/// The constructor starts the dispatching loop automatically. New tasks
/// are push into the task queue using the function call operator.
/// The destructor blocks and waits until all tasks are dispatched.
template <bool hasExternalTaskQueue>
class GenericTaskQueueThread
    : private detail::GenericTaskQueueThreadData<hasExternalTaskQueue>
{
private:
  using Base = detail::GenericTaskQueueThreadData<hasExternalTaskQueue>;
  std::thread worker;

  void startWorker()
  {
    worker = std::thread( [this]()
    {
      while (!this->done)
      {
        this->queue.popAndExecute();
      }
    });
  }

public:
  /// Starts the task dispatching loop an another thread.
  template <bool B = hasExternalTaskQueue,
            typename std::enable_if<!B,int>::type = 0>
  explicit GenericTaskQueueThread()
  {
    startWorker();
  }

  /// Starts the task dispatching loop an another thread.
  template <bool B = hasExternalTaskQueue,
            typename std::enable_if<B,int>::type = 0>
  explicit GenericTaskQueueThread( TaskQueue & queue )
    : Base( queue )
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
    (*this)([this](){ this->done = true; });
    worker.join();
  }
};

using TaskQueueThread         = GenericTaskQueueThread<false>;
using ExternalTaskQueueThread = GenericTaskQueueThread<true >;


namespace detail
{
  template <typename T>
  class ScopedArray // fixed size, for non-copyable and non-movable types
  {
  private:
    std::size_t n = 0;
    T * data = nullptr;

    void destroy( std::size_t i )
    {
      while ( --i != std::size_t(-1) )
      {
        data[i].~T();
      }
    }

    ScopedArray() = default;

  public:
    template <typename ...Args>
    ScopedArray( std::size_t nElems, Args &&... args )
      : ScopedArray()
    {
      if ( nElems == 0 )
        return;

      data = (T*)::operator new[]( nElems * sizeof(T) );
      for ( ; n < nElems; ++n )
        new ((void*)(data+n))T( std::forward<Args>(args)... );
    }

    ~ScopedArray()
    {
      destroy( n );
      delete[] data;
    }
  };

} // detail

/// A task dispatching thread class.
///
/// The constructor starts the dispatching loop automatically. New tasks
/// are push into the task queue using the function call operator.
/// The destructor blocks and waits until all tasks are dispatched.
class TaskQueueThreadPool
{
private:
  TaskQueue queue;
  detail::ScopedArray<ExternalTaskQueueThread> workers;

  auto computeNWorkers( std::size_t nThreads )
  {
    if ( nThreads != 0 )
      return nThreads;

    return std::max( std::size_t{1}, std::thread::hardware_concurrency() );
  }

public:
  /// Starts the task dispatching loop an another thread.
  TaskQueueThreadPool( const std::size_t nThreads = 0 )
    : workers( computeNWorkers(nThreads), queue )
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
