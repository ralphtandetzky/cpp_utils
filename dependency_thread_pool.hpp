#pragma once

#include "scope_guard.hpp"
#include "task_queue_thread_pool.hpp"

#include <algorithm>
#include <cassert>
#include <future>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace cu
{

class DependencyThreadPoolBase
{
public:
  using Id = std::size_t;
  static constexpr const Id invalidId = -1;

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


template <typename ...Args>
class DependencyThreadPool
    : public DependencyThreadPoolBase
{
public:
  template <typename F>
  Result<typename std::result_of<F(Args...)>::type> operator()(
      std::vector<Id> dependencyIds,
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
        const std::vector<Id> & dependencies,
        Id id )
    {
      auto nDependencies = 0;
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
    assert( nodeIt->nOpenDependencies == 0 );
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
