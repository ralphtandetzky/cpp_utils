#pragma once

#include <mutex>

namespace cu
{

struct PassUniqueLockTag {};

template <typename T>
class Monitor
{
private:
  T item;
  mutable std::mutex mutex;

public:
  template <typename ...Args>
  Monitor( Args &&... args )
    : item( std::forward<Args>(args)... )
  {
  }

  template <typename F>
  auto operator()( F && f )
  {
    std::lock_guard<std::mutex> lock(mutex);
    return std::forward<F>(f)( item );
  }

  template <typename F>
  auto operator()( F && f ) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return std::forward<F>(f)( item );
  }

  template <typename F>
  auto operator()( PassUniqueLockTag, F && f )
  {
    std::unique_lock<std::mutex> lock(mutex);
    return std::forward<F>(f)( item, lock );
  }

  template <typename F>
  auto operator()( PassUniqueLockTag, F && f ) const
  {
    std::unique_lock<std::mutex> lock(mutex);
    return std::forward<F>(f)( item, lock );
  }
};

} // namespace cu
