// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

/** @file Defines the Updater class.
 * @author Ralph Tandetzky
 */

#pragma once

#include "functors.hpp"
#include "monitor.hpp"
#include <condition_variable>

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
template <typename Executor, typename ... TaskArgs>
class Updater
{
private:
  Executor executor;

  struct Data
  {
    MoveFunction<void(TaskArgs&&...)> task;
    bool running = false;
    bool done = false;
    std::condition_variable condition;
  };
  Monitor<Data> data;

  void runExecutor()
  {
    executor( [this]( TaskArgs ... taskArgs )
    {
      MoveFunction<void(TaskArgs&&...)> task;
      data( [&]( Data & data )
      {
        assert( data.running );
        task.swap( data.task );
      });

      if ( task )
        task( std::forward<TaskArgs>(taskArgs)... );

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
    (*this)( [this]( const TaskArgs &... )
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
    MoveFunction<void(TaskArgs&&...)> task(std::forward<F>(f));
    const auto previouslyRunning = data( [&]( Data & data )
    {
      task.swap( data.task );
      const auto previouslyRunning = data.running;
      data.running = true;
      return previouslyRunning;
    } );
    if ( !previouslyRunning )
      runExecutor();
  }
};

} // namespace cu
