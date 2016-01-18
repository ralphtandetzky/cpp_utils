#pragma once

#include "math_helpers.hpp"

#include <array>
#include <cassert>
#include <tuple>

namespace cu
{

template <typename T, std::size_t N>
class Point
{
public:
  template <typename ...Us, typename = decltype(std::array<T,N>{std::declval<Us>()...})>
  Point( Us && ...args )
    : x_{ std::forward<Us>(args)... }
  {}

  T &operator[]( std::size_t idx )
  {
    return x_[idx];
  }

  const T & operator[]( std::size_t idx ) const
  {
    return x_[idx];
  }

private:
  std::array<T,N> x_;
};


template <typename T, std::size_t N>
class Rect
{
  using Point = Point<T,N>;
public:
  Rect()
  {}

  Rect( Point A, Point B )
    : A_(A)
    , B_(B)
  {
    checkInvariants();
  }

  constexpr Point A() const { return A_; }
  constexpr Point B() const { return B_; }

  void setA( const Point & P ) { A_ = P; checkInvariants(); }
  void setB( const Point & P ) { B_ = P; checkInvariants(); }

private:
  void checkInvariants() const
  {
    for ( std::size_t i = 0; i != N; ++i )
      assert( A_[i] <= B_[i] );
  }

  Point A_;
  Point B_;
};


namespace detail
{
  template <typename T>
  std::pair<T,T> intersect_interval( const T & a1, const T & b1,
                                     const T & a2, const T & b2 )
  {
    if ( b1 <= a2 || a1 >= b2 )
      return { {}, {} };
    return { std::max( a1, a2 ), std::min(b1, b2) };
  }

  template <typename T, std::size_t N, std::size_t ...indices>
  Rect<T,N> intersect_impl( const Rect<T,N> & lhs,
                            const Rect<T,N> & rhs,
                            std::index_sequence<indices...> )
  {
    Point<T,N> A;
    Point<T,N> B;
    for ( std::size_t i = 0; i != N; ++i )
    {
      std::tie( A[i], B[i] ) = intersect_interval(
            lhs.A()[i], lhs.B()[i],
            rhs.A()[i], rhs.B()[i] );
    }
    return { A, B };
  }
}

template <typename T, std::size_t N>
Rect<T,N> operator &( const Rect<T,N> & lhs, const Rect<T,N> & rhs )
{
  return detail::intersect_impl( lhs, rhs, std::make_index_sequence<N>() );
}


} // namespace cu
