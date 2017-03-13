// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include "rank.hpp"

#include <functional>
#include <utility>

// a non cu-namespace is used in order to avoid infinite recursion.
namespace cu_swap_impl
{
  template <typename T>
  struct is_std_function : std::false_type {};
  template <typename T>
  struct is_std_function<std::function<T>> : std::true_type {};

  template <typename T>
  auto swap_impl( T & lhs, T & rhs, cu::Rank<1> ) noexcept
    -> decltype( lhs.swap(rhs) )
  {
    static_assert(
          // workaround for GCC-Bug 77322 -
          // [C++11] std::function::swap should be noexcept.
          is_std_function<std::decay_t<T>>::value ||
          noexcept( lhs.swap(rhs) ), "Swap is not noexcept!" );
    return lhs.swap(rhs);
  }

  template <typename T>
  void swap_impl( T & lhs, T & rhs, cu::Rank<0> ) noexcept
  {
    using std::swap;
    static_assert( noexcept( swap(lhs,rhs) ), "Swap is not noexcept!" );
    swap(lhs,rhs);
  }
} // namespace cu_swap_impl

namespace cu
{

/// The ultimate swapperator! ;)
///
/// Exchanges the values of @c lhs and @c rhs. If @c T is a class with a
/// @c swap() member function, then this will be chosen for swapping the
/// parameters. Otherwise, if there's a swap defined in the namespace @c std
/// or the namespace of @c T, then that swap will be used to swap the
/// values. In any case, the used operations are checked to be @c noexcept.
/// In this manner, this swap function provides the @c noexception guarantee.
///
/// Note that two template arguments are used, so that there is no ambiguity,
/// when swapping objects of a type in the @c cu namespace that does not have
/// its own @c cu::swap() overload.
template <typename T1,
          typename T2>
void swap( T1 & lhs, T2 & rhs ) noexcept
{
  ::cu_swap_impl::swap_impl( lhs, rhs, Rank<1>() );
}

/// Swapping of two built-in arrays of equal size.
///
/// Internally the ultimate swapperator is used to perform the task.
template <typename T1,
          typename T2,
          std::size_t N>
void swap( T1(&lhs)[N], T2(&rhs)[N] ) noexcept
{
  for ( std::size_t i = 0; i != N; ++i )
    ::cu::swap( lhs[i], rhs[i] );
}

} // namespace cu
