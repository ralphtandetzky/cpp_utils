#pragma once

#include "monitor.hpp"

#include <cassert>
#include <condition_variable>
#include <list>
#include <type_traits>

namespace cu
{

template <typename T>
class ConcurrentQueue
{
public:
  void push( const T & item )
  {
    emplace( item );
  }

  void push( T && item )
  {
    emplace( std::move(item) );
  }

  template <typename ...Args>
  void emplace( Args &&... args )
  {
    std::list<T> l;
    l.emplace_back( std::forward<Args>(args)... );
    data( [&l]( Data & data )
    {
      data.items.splice( data.items.end(), std::move(l) );
      data.condition.notify_one();
    });
  }

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
