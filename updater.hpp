/** @file Defines the Updater class.
 * @author Ralph Tandetzky
 */

#pragma once

#include "monitor.hpp"
#include <condition_variable>
#include <functional>

namespace cu
{

/// A class wrapping an executor for executing updating operations.
///
/// If there might be too many expensive updating tasks for a
/// task queue to handle, then it might be desirable to drop some tasks
/// in order to not jam up the task queue. This class does just that.
/// If there is already a task scheduled and a new one comes in,
/// then the scheduled task is replaced by the new one which will be executed
/// in its stead.
template <typename Executor>
class Updater
{
private:
  Executor executor;

  struct Data
  {
    std::function<void()> task;
    bool running = false;
    bool done = false;
    std::condition_variable condition;
  };
  Monitor<Data> data;

  void runExecutor()
  {
    executor( [this]()
    {
      std::function<void()> task;
      data( [&]( Data & data )
      {
        assert( data.running );
        task.swap( data.task );
      });

      if ( task )
        task();

      const auto runAgain = data( []( Data & data )
      {
        assert( data.running );
        if ( data.task )
          return true;
        data.running = false;
        return false;
      });

      if ( runAgain )
        runExecutor();
    });
  }

public:
  /// Forwards the arguments to the constructor of the @c Executor.
  template <typename ...Args>
  Updater( Args &&... args )
    : executor( std::forward<Args>(args)... )
  {
  }

  /// Waits until all tasks are done.
  ~Updater()
  {
    // The following is executed unconditionally, since the client of the
    // class cannot push further tasks that might overwrite this one.
    (*this)( [this]()
    {
      data( []( Data & data )
      {
        data.done = true;
        data.condition.notify_one();
      });
    });

    data( PassUniqueLockTag(),
          []( Data & data, std::unique_lock<std::mutex> & lock )
    {
      data.condition.wait( lock, [&]{ return data.done; } );
    });
  }

  /// Pushes an updating task.
  ///
  /// The task might actually not be executed, if further tasks are queued.
  template <typename F>
  void operator()( F && f )
  {
    std::function<void()> task(std::forward<F>(f));
    const auto previouslyRunning = data( [&]( Data & data )
    {
      task.swap( data.task );
      const auto previouslyRunning = data.running;
      data.running = true;
      return previouslyRunning;
    });
    if ( !previouslyRunning )
      runExecutor();
  }
};

} // namespace cu
