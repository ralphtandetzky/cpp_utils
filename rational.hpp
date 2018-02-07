#pragma once

#include <tuple>
#include <type_traits>

namespace cu
{

/// Calculates the greatest common denominator of two integers.
template <typename I>
I gcd( I a, I b )
{
  assert( a >= I(0) );
  assert( b >= I(0) );

  if ( b > a )
    std::swap( a, b );

  while ( b != I(0) )
    std::tie( a, b ) = std::make_tuple( b, a % b );

  return a;
}


/// Class implementing exact rational number arithmetic.
///
/// The template parameter @c I should be an integer type like @c int or
/// @c std::int64_t.
///
/// Note that @c Rational<I> does not keep numerator and denominator prime
/// to each other. This is for performance reasons.
/// The user must ensure that the numerator and denominator stay within the
/// bounds of the undelying integer type @c I.
/// To get the reduced fraction, call @c normalized().
template <typename I>
class Rational
{
  using BoolType = void(*)();

public:
  /// Constructs a fraction with value 0.
  constexpr Rational() noexcept = default;

  /// Constructs a fraction with numerator @c val and denominator @c 1.
  constexpr Rational( I val ) noexcept
    : p( val )
  {}

  /// @pre The denominator must not be @c 0.
  constexpr Rational( I numerator, I denominator ) noexcept
    : p( numerator )
    , q( denominator )
  {}

  template <typename J>
  constexpr Rational( const Rational<J> & other )
    : p( other.p )
    , q( other.q )
  {}

  Rational( float ) = delete;
  Rational( double ) = delete;

  constexpr I num() const noexcept
  {
    return p;
  }

  constexpr I den() const noexcept
  {
    return q;
  }

  constexpr Rational normalized() noexcept
  {
    const auto d = gcd( std::abs(p), std::abs(q) );
    if ( d == I(1) )
      return *this;
    else
      return { p / d, q / d };
  }

  constexpr void normalize() noexcept
  {
    *this = normalized( *this );
  }

  constexpr Rational & operator+=( const Rational & other ) noexcept
  {
    if ( q == other.q )
    {
      p += other.p;
    }
    else
    {
      std::tie( p, q ) = {
            p * other.q +
            q * other.p,
            q * other.q };
    }
    return *this;
  }

  constexpr Rational & operator+=( I val ) noexcept
  {
    p += val * q;
    return *this;
  }

  constexpr Rational & operator-=( const Rational & other ) noexcept
  {
    if ( q == other.q )
    {
      p -= other.p;
    }
    else {
      std::tie( p, q ) = {
            p * other.q -
            q * other.p,
            q * other.q };
    }

    return *this;
  }

  constexpr Rational & operator-=( I val ) noexcept
  {
    p -= val * q;
    return *this;
  }

  constexpr Rational & operator*=( const Rational & other ) noexcept
  {
    p *= other.p;
    q *= other.q;
    return *this;
  }

  constexpr Rational & operator*=( I val ) noexcept
  {
    p *= val;
    return *this;
  }

  /// @pre Other must not be @c 0.
  constexpr Rational & operator/=( const Rational & other ) noexcept
  {
    p *= other.q;
    q *= other.p;
    return *this;
  }

  constexpr Rational & operator/=( I val ) noexcept
  {
    q *= val;
    return *this;
  }

  constexpr Rational & operator++() noexcept
  {
    p += q;
  }

  constexpr Rational operator++(int) noexcept
  {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  constexpr Rational & operator--() noexcept
  {
    p -= q;
  }

  constexpr Rational operator--(int) noexcept
  {
    auto tmp = *this;
    --this;
    return tmp;
  }

  constexpr bool operator<( const Rational & other ) const noexcept
  {
    return (q > 0) == (other.q > 0) ?
          p * other.q < other.p * q :
          p * other.q > other.p * q;
  }

  constexpr bool operator==( const Rational & other ) const noexcept
  {
      return p * other.q == other.p * q;
  }

  constexpr operator BoolType() const noexcept
  {
    return p ? &boolTypeFunc : nullptr;
  }

  constexpr bool operator!() const noexcept
  {
    return !p;
  }

  constexpr Rational operator+() const noexcept
  {
    return *this;
  }

  constexpr Rational operator-() const noexcept
  {
    return { -p, q };
  }

  constexpr I rounded() const noexcept
  {
    return (p >= 0) == (q > 0) ?
        ( p + q/I(2) ) / q :
        ( p - q/I(2) ) / q;
  }

private:
  static void boolTypeFunc() {} // dummy function to provide BoolType

  I p = I(0);
  I q = I(1); // always positive

  template <typename J>
  friend class Rational;
};

template <typename I>
constexpr Rational<I> operator+( Rational<I> lhs, const Rational<I> & rhs )
{
  return lhs += rhs;
}

template <typename I>
constexpr Rational<I> operator+( Rational<I> lhs, I rhs )
{
  return lhs += rhs;
}

template <typename I>
constexpr Rational<I> operator+( I lhs, Rational<I> rhs )
{
  return rhs += lhs;
}

template <typename I>
constexpr Rational<I> operator-( Rational<I> lhs, const Rational<I> & rhs )
{
  return lhs -= rhs;
}

template <typename I>
constexpr Rational<I> operator-( Rational<I> lhs, I rhs )
{
  return lhs -= rhs;
}

template <typename I>
constexpr Rational<I> operator-( I lhs, Rational<I> rhs )
{
  return -(rhs -= lhs);
}

template <typename I>
constexpr Rational<I> operator*( Rational<I> lhs, const Rational<I> & rhs )
{
  return lhs *= rhs;
}

template <typename I>
constexpr Rational<I> operator*( Rational<I> lhs, I rhs )
{
  return lhs *= rhs;
}

template <typename I>
constexpr Rational<I> operator*( I lhs, Rational<I> rhs )
{
  return rhs *= lhs;
}

template <typename I>
constexpr Rational<I> operator/( Rational<I> lhs, const Rational<I> & rhs )
{
  return lhs /= rhs;
}

template <typename I>
constexpr Rational<I> operator/( Rational<I> lhs, I rhs )
{
  return lhs /= rhs;
}

template <typename I>
constexpr Rational<I> operator/( I lhs, Rational<I> rhs )
{
  return Rational<I>{lhs} /= rhs;
}

template <typename I>
constexpr bool operator>( const Rational<I> & lhs, const Rational<I> & rhs )
{
  return rhs < lhs;
}

template <typename I>
constexpr bool operator<=( const Rational<I> & lhs, const Rational<I> & rhs )
{
  return !(rhs < lhs);
}

template <typename I>
constexpr bool operator>=( const Rational<I> & lhs, const Rational<I> & rhs )
{
  return !(lhs < rhs);
}

template <typename I>
constexpr bool operator!=( const Rational<I> & lhs, const Rational<I> & rhs )
{
  return !(rhs == lhs);
}

} // namespace cu
