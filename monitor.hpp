/** @file Defines the template class @c cu::Monitor which implements the
 * monitor pattern.
 *
 * @author Ralph Tandetzky
 */

#pragma once

#include "swap.hpp"

#include <mutex>

namespace cu
{

/// Tag class which indicates to a function that it should pass an
/// @c std::unique_lock to a functor.
struct PassUniqueLockTag {};

/// A wrapper which makes reentrant types thread-safe by the monitor pattern.
///
/// This class statically ensures that the wrapped object is only accessible
/// when the responsible mutex is locked. Access is done by passing a functor
/// to the function call operator. The function call operator locks the
/// mutex and calls the functor passing a reference to the wrapped to it.
///
/// This class makes the error-prone explicit use of mutexes unneccesary.
/// Use this class instead of using @c std::mutex directly.
///
/// For the use of condition variables @c std::unique_lock is needed.
/// This can be achieved by passing a @c PassUniqueLockTag to the
/// function call operator of the monitor, which will then call the
/// functor with an additional reference to @c std::unique_lock as argument.
///
/// Example: An atomic counter can be implemented like this:
///   @code
///     class AtomicCounter
///     {
///     public:
///         AtomicCounter( int i = 0 )
///             : val(i)
///         {}
///
///         int get() const
///         {
///             return val( []( auto val ){ return val } );
///         }
///
///         int operator++()
///         {
///             return val( []( auto & val ){ return ++val; } );
///         }
///
///     private:
///         Monitor<int> val;
///     };
///   @endcode
/// Of course in this case, it would make more sense to use std::atomic<int>,
/// but this example shows the simplicity of @c cu::Monitor's use.
template <typename T>
class Monitor
{
private:
  T item;
  mutable std::mutex mutex;

public:
  /// Forwards all arguments to the wrapped type's constructor.
  template <typename ...Args>
  Monitor( Args &&... args )
    : item( std::forward<Args>(args)... )
  {
  }

  /// Locks the mutex and applies the passed functor to the wrapped item.
  ///
  /// @returns whatever the functor returns.
  template <typename F>
  decltype(auto) operator()( F && f )
  {
    std::lock_guard<std::mutex> lock(mutex);
    return std::forward<F>(f)( item );
  }

  /// Locks the mutex and applies the passed functor to the wrapped item.
  ///
  /// @returns whatever the functor returns.
  template <typename F>
  decltype(auto) operator()( F && f ) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return std::forward<F>(f)( item );
  }

  /// The same as the normal function call operator, but the locking
  /// @c std::unique_lock is passed to the given functor.
  template <typename F>
  decltype(auto) operator()( PassUniqueLockTag, F && f )
  {
    std::unique_lock<std::mutex> lock(mutex);
    return std::forward<F>(f)( item, lock );
  }

  /// The same as the normal function call operator, but the locking
  /// @c std::unique_lock is passed to the given functor.
  template <typename F>
  decltype(auto) operator()( PassUniqueLockTag, F && f ) const
  {
    std::unique_lock<std::mutex> lock(mutex);
    return std::forward<F>(f)( item, lock );
  }

  /// Atomically swaps the contents of @c this object with the @c other object.
  void exchange( T & other )
  {
    (*this)( [&]( T & mine ){ ::cu::swap( mine, other ); } );
  }

  void exchange( T && other )
  {
    exchange( other );
  }

  /// Returns the contents of this object.
  T load() const
  {
    return (*this)( []( auto && item ){ return item; } );
  }
};

} // namespace cu
