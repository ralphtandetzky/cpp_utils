// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

/** @file Defines the class cu::Concurrent.
 * @author Ralph Tandetzky
 */

#pragma once

#include "functors.hpp"

#include <memory>
#include <utility>

namespace cu
{

/// Wraps the type T, so that all accesses are asynchroneous.
///
/// The @c Executor template argument should be a class that implements
/// an @c operator()() which takes a functor to be executed as an argument
/// and returns a future to the respective result type. The passed functors
/// take the wrapped item as first argument and possibly further arguments
/// that are passed from the executor. One very simple executor is the
/// @c cu::TaskQueueThread which does not pass any arguments to the
/// functor it takes. Hence the functor parameter
/// of @c Concurrent<T,TaskQueueThread>::operator()() must take
/// a parameter of type @c T& or @c const T&.
///
/// Alternatively, a reference type like @c cu::TaskQueueThread& can be
/// chosen as executor, or, types which distribute the work on threadpools.
///
/// A typical way of using this class is to have a member variable of a class
/// wrapped by @c cu::Concurrent. In order to group several variables,
/// a simple C-style struct may be used.
///   @code
///     struct Data
///     {
///         int x = 0, y = 0;
///     };
///     cu::Concurrent<Data,cu::TaskQueueThread> data;
///   @endcode
/// The actual data can then be accessed the following way:
///   @code
///     std::future<int> MyClass::getX() const
///     {
///         return data( []( const auto & data ){ return data.x; } );
///     }
///   @endcode
template <typename T, typename Executor>
class Concurrent
{
private:
  T item; // wrapped object
  Executor executor; // executed functors

public:
  /// Constructs the wrapped type asynchroneously.
  template <typename ...Args>
  Concurrent( Executor executor_, Args &&... args )
    : item( std::forward<Args>(args)...)
    , executor( std::forward<Executor>(executor_) )
  {
  }

  /// Waits for all operations to be completed and then destroys the wrapped
  /// object.
  ~Concurrent()
  {
    // block until all tasks are finished.
    executor( NoOpFunctor() ).wait();
  }

  /// Simplified: Essentially calls runs @c f(item) through the executor.
  ///
  /// If the executor passes arguments to its functors, then these are passed
  /// to @c f as additional arguments.
  ///
  /// @returns the future that is returned by the executor.
  template <typename F>
  auto operator()( F && f )
  {
    return executor( std::bind(std::forward<F>(f),std::ref(item)) );
  }

  /// Simplified: Essentially calls runs @c f(item) through the executor.
  ///
  /// Same as the non-const variant, but a @c const item is bound as the
  /// first argument to the functor that is dispatched by the executor.
  template <typename F>
  auto operator()( F && f ) const
  {
    return executor( std::bind(std::forward<F>(f),std::ref(item)) );
  }
};


} // namespace cu
