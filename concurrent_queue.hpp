/** @file Defines the class cu::ConcurrentQueue.
 * @author Ralph Tandetzky
 */

#pragma once

#include "monitor.hpp"

#include <cassert>
#include <condition_variable>
#include <list>
#include <type_traits>

namespace cu
{

/// High-performance thread-safe queue.
///
/// New elements can be added using the member functions @c push()
/// and @c emplace(). Elements can be retrieved by the blocking member
/// function @c pop().
///
/// The implementation supports arbitrary numbers of consumer and
/// producer threads and should scale very well, since mutexes are
/// only locked for very short times.
///
/// The type @c T must be movable (or copyable) for the class to work.
template <typename T>
class ConcurrentQueue
{
public:
  /// Copies an item into the queue.
  ///
  /// This function provides the strong exception guarantee.
  void push( const T & item )
  {
    emplace( item );
  }

  /// Moves an item into the queue.
  ///
  /// This function provides the strong exception guarantee.
  void push( T && item )
  {
    emplace( std::move(item) );
  }

  /// Emplaces an item into the queue, i. e. constructor arguments are forwarded.
  ///
  /// This function provides the strong exception guarantee.
  template <typename ...Args>
  void emplace( Args &&... args )
  {
    std::list<T> l;
    l.emplace_back( std::forward<Args>(args)... );
    data( [&l]( Data & data )
    {
      // commit
      data.items.splice( data.items.end(), std::move(l) );
      data.condition.notify_one(); // does not throw.
    });
  }

  /// Pops an item from the queue and returns it.
  ///
  /// If there's no item in the queue, then the function will block until there
  /// is one.
  ///
  /// This function provides the strong exception guarantee, if
  /// the move constructor of @c T does not throw. Otherwise,
  /// the basic exception guarantee holds.
  T pop()
  {
    std::list<T> l;
    data( PassUniqueLockTag(), [&l]( Data & data, std::unique_lock<std::mutex> & lock )
    {
      data.condition.wait( lock, [&](){ return !data.items.empty(); } );
      l.splice( l.end(), data.items, data.items.begin() );
    });

    assert( !l.empty() );
    return std::move( l.front() );
  }

private:
  struct Data
  {
    std::list<T> items;
    std::condition_variable condition;
  };

  Monitor<Data> data;
};

}
