#pragma once

#include <memory>
#include <utility>

namespace cu
{

/// Wraps the type T, so that all accesses are asynchroneous.
template <typename T, typename Executor>
class Concurrent
{
private:
  T item;
  Executor executor;

public:
  /// Constructs the wrapped type asynchroneously.
  template <typename ...Args>
  Concurrent( Executor executor_, Args &&... args )
    : item( std::forward<Args>(args)...)
    , executor( std::forward<Executor>(executor_) )
  {
  }

  ~Concurrent()
  {
    // block until all tasks are finished.
    executor( [](){} ).wait();
  }

  template <typename F>
  auto operator()( F && f )
  {
    return executor( std::bind(std::forward<F>(f),std::ref(item)) );
  }

  template <typename F>
  auto operator()( F && f ) const
  {
    return executor( std::bind(std::forward<F>(f),std::ref(item)) );
  }
};


} // namespace cu
