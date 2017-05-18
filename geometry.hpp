#pragma once

#include "math_helpers.hpp"

#include <array>
#include <cassert>
#include <tuple>

namespace cu
{

/// An N-dimensional point whose coordinate type is @c T.
///
/// The coordinates can be accessed via the @c operator[]().
/// The constructor takes the same arguments as an @c std::array<T,N>.
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

  bool operator==( const Point & other )
  {
    return x_ == other.x_;
  }

private:
  std::array<T,N> x_;
};


/// An @c N-dimensional rectangle whose coordinate type is @c T.
///
/// The rectangle is determined by two points @c A and @c B, where no
/// coordinate of @c A may be larger than the corresponding coordinate
/// of @c B. This ensures, that the edges of the rectangle all have
/// non-negative lenghts.
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
    if ( b1 < a2 || a1 > b2 )
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

/// Calculates the intersection of two rectangles.
///
/// If the intersection is empty, then the resulting rectangle will have
/// volume @c 0.
template <typename T, std::size_t N>
Rect<T,N> operator &( const Rect<T,N> & lhs, const Rect<T,N> & rhs )
{
  return detail::intersect_impl( lhs, rhs, std::make_index_sequence<N>() );
}

template <typename T, std::size_t N>
bool operator==( const Rect<T,N> & lhs, const Rect<T,N> & rhs )
{
  return lhs.A() == rhs.A() && lhs.B() == rhs.B();
}


} // namespace cu
