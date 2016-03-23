#pragma once

#include "meta_programming.hpp"
#include <utility>

namespace cu
{

namespace detail
{
  template <typename T>
  auto swap_impl( T & lhs, T & rhs, Rank<1> ) noexcept
    -> decltype( lhs.swap(rhs) )
  {
    static_assert( noexcept( lhs.swap(rhs) ), "Swap is not noexcept!" );
    return lhs.swap(rhs);
  }

  template <typename T>
  void swap_impl( T & lhs, T & rhs, Rank<0> ) noexcept
  {
    using std::swap;
    static_assert( noexcept( swap(lhs,rhs) ), "Swap is not noexcept!" );
    swap(lhs,rhs);
  }
} // namespace detail

/// The ultimate swapperator! ;)
///
/// Exchanges the values of @c lhs and @c rhs. If @c T is a class with a
/// @c swap() member function, then this will be chosen for swapping the
/// parameters. Otherwise, if there's a swap defined in the namespace @c std
/// or the namespace of @c T, then that swap will be used to swap the
/// values. In any case, the used operations are checked to be @c noexcept.
/// In this manner, this swap function provides the @c noexception guarantee.
template <typename T>
void swap( T & lhs, T & rhs ) noexcept
{
  detail::swap_impl( lhs, rhs, Rank<1>() );
}

/// Swapping of two built-in arrays of equal size.
///
/// Internally the ultimate swapperator is used to perform the task.
template <typename T, std::size_t N>
void swap( T(&lhs)[N], T(&rhs)[N] ) noexcept
{
  for ( std::size_t i = 0; i != N; ++i )
    ::cu::swap( lhs[i], rhs[i] );
}

} // namespace cu
