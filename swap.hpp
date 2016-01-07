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

template <typename T>
void swap( T & lhs, T & rhs ) noexcept
{
  detail::swap_impl( lhs, rhs, Rank<1>() );
}

template <typename T, std::size_t N>
void swap( T(&lhs)[N], T(&rhs)[N] ) noexcept
{
  for ( std::size_t i = 0; i != N; ++i )
    ::cu::swap( lhs[i], rhs[i] );
}

} // namespace cu
