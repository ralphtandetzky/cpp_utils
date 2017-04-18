// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include "scope_guard.hpp"
#include "slice.hpp"
#include "task_queue_thread_pool.hpp"

#include <algorithm>
#include <cassert>
#include <future>
#include <map>
#include <type_traits>
#include <unordered_map>

namespace cu
{

class DependencyThreadPoolBase
{
public:
  using Id = std::size_t;
  static constexpr const Id invalidId = ~static_cast<Id>(0);

  template <typename T>
  struct Result
  {
    Result() = default;

    Result( std::future<T> future_, Id id_ )
      : future( std::move(future_) )
      , id( id_ )
    {}

    std::future<T> future;
    Id id = invalidId;
  };
};


/// A threadpool for tasks that depend upon each other.
///
/// The threadpool will only start the execution of a task, if all the tasks
/// it depends on have been finished.
/// When the client issues a task, then an @c std::future<T> and a task id
/// will be returned in a @c DependencyThreadPoolBase::Result<T> structure.
/// The returned id can be used as dependency for other subsequent tasks.
/// Therefore cyclic dependencies are impossible by design.
///
/// The template type arguments are the arguments that will be passed to the
/// tasks and are data elements of the threadpool just as in
/// @c TaskQueueThreadPool.
template <typename ...Args>
class DependencyThreadPool
    : public DependencyThreadPoolBase
{
public:
  /// Schedules a task.
  ///
  /// The task will be executed only after all tasks it depends upon have
  /// been finished.
  /// @param dependencyIds specifies the tasks upon which the new task depends.
  /// @returns A struct with a member @c future which designates the
  /// asynchroneous result and a member @c id which identifies the task with a
  /// unique id. This id can be used as dependency id for new tasks.
  template <typename F>
  Result<typename std::result_of<F(Args...)>::type> operator()(
      const cu::Slice<const Id> dependencyIds,
      F && f )
  {
    auto pt = std::packaged_task<std::result_of_t<F(Args...)>(Args&&...)>(
          std::forward<F>(f) );
    auto future = pt.get_future();
    cu::MoveFunction<void(Args&&...)> task = std::move(pt);
    const auto id = data( [&]( Data & data )
    {
      const auto id = data.idCounter;
      ++data.idCounter;
      const auto nDependencies = data.updateDependencies( dependencyIds, id );
      data.nodes.insert(
        std::make_pair( id, Node{ nDependencies, {}, std::move(task) } ) );
      if ( nDependencies == 0 )
        queueTask( id, data.nodes );
      return id;
    } );
    return { std::move(future), id };
  }

private:
  struct Node
  {
    Node(
        std::size_t nOpenDependencies_,
        std::vector<Id> dependentTasks_,
        cu::MoveFunction<void(Args&&...)> task_ )
      : nOpenDependencies( nOpenDependencies_ )
      , dependentTasks( std::move( dependentTasks_ ) )
      , task( std::move( task_ ) )
    {}

    std::size_t nOpenDependencies;
    std::vector<Id> dependentTasks;
    cu::MoveFunction<void(Args&&...)> task;
  };

  struct Data
  {
    Id idCounter = 0;
    std::map<Id,Node> nodes;

    std::size_t updateDependencies(
        const cu::Slice<const Id>  dependencies,
        Id id )
    {
      std::size_t nDependencies = 0;
      for ( auto dep : dependencies )
      {
        const auto it = nodes.find( dep );
        if ( it == nodes.end() )
          continue;
        it->second.dependentTasks.push_back( id );
        ++nDependencies;
      }
      return nDependencies;
    }
  };

  void queueTask( Id id, std::map<Id,Node> & nodes )
  {
    const auto nodeIt = nodes.find(id);
    assert( nodeIt != nodes.end() );
    assert( nodeIt->second.nOpenDependencies == 0 );
    workers( [this, nodeIt]
             ( Args &&... args ) mutable
    {
      CU_SCOPE_EXIT
      {
        this->data( [&]( Data & data )
        {
          for ( auto dependentId : nodeIt->second.dependentTasks )
          {
            const auto dependentNodeIt = data.nodes.find( dependentId );
            assert( dependentNodeIt != data.nodes.end() );
            const auto nRemainingDependencies =
                --dependentNodeIt->second.nOpenDependencies;
            if ( nRemainingDependencies == 0 )
              queueTask( dependentId, data.nodes );
          }
          data.nodes.erase( nodeIt );
        } );
      };
      nodeIt->second.task( std::forward<Args>(args)... );
    } );
  }

  cu::Monitor<Data> data;
  TaskQueueThreadPool<Args...> workers;
};

} // namespace cu
