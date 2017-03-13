// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include <array>
#include <cassert>
#include <cstdint>

namespace cu
{

template <typename T, std::size_t N>
class Polynomial
{
public:
  Polynomial( T val = 0 )
  {
    coeffs[0] = val;
  }

  template <typename U,
            std::size_t M,
            typename std::enable_if<
              std::is_convertible<U,T>::value && M <= N,int>::type = 0>
  Polynomial( const Polynomial<U,M> & other )
  {
    for ( std::size_t m = 0; m <= M; ++m )
      coeffs[m] = other[m];
  }

  T & operator[]( std::size_t n )
  {
    assert( n <= N );
    return coeffs[n];
  }

  T operator[]( std::size_t n ) const
  {
    assert( n <= N );
    return coeffs[n];
  }

  T operator()( T x ) const
  {
    auto val = coeffs[N];
    for ( std::size_t i = 1; i <= N; ++i )
    {
      val *= x;
      val += coeffs[N-i];
    }
    return val;
  }

private:
  std::array<T,N+1> coeffs{};
};

template <typename T, std::size_t N>
Polynomial<T,N> & operator*=( Polynomial<T,N> & p, T val )
{
  for ( std::size_t index = 0; index <= N; ++index )
    p[index] *= val;
  return p;
}

template <typename T, std::size_t N>
auto operator*( Polynomial<T,N> p, T val )
{
  return std::move( p *= val );
}

template <typename T, std::size_t N>
auto operator*( T val, Polynomial<T,N> p )
{
  return std::move( p *= val );
}

template <typename T, std::size_t M, std::size_t N>
Polynomial<T,M+N> operator*( const Polynomial<T,M> & p,
                             const Polynomial<T,N> & q )
{
  Polynomial<T,M+N> pq;

  for ( std::size_t k = 0; k <= M+N; ++k )
  {
    const std::size_t m_min = k <= N ? 0 : k-N;
    const std::size_t m_max = k <= M ? k : M;
    for ( std::size_t m = m_min; m <= m_max; ++m )
      pq[k] += p[m]*q[k-m];
  }

  return pq;
}

template <typename T, std::size_t N>
Polynomial<T,N> & operator/=( Polynomial<T,N> & p, T val )
{
  return p *= (1/val);
}

template <typename T, std::size_t N>
auto operator/( Polynomial<T,N> p, T val )
{
  return std::move( p /= val );
}

template <typename T, std::size_t M, std::size_t N>
typename std::enable_if<(M>=N),Polynomial<T,M> >::type
      operator+( const Polynomial<T,M> & p, const Polynomial<T,N> & q )
{
  auto p_copy = p;
  for ( std::size_t n = 0; n <= N; ++n )
    p_copy[n] += q[n];

  return std::move( p_copy );
}

template <typename T, std::size_t M, std::size_t N>
typename std::enable_if<(M<N),Polynomial<T,N> >::type
      operator+( const Polynomial<T,M> & p, const Polynomial<T,N> & q )
{
  return q + p;
}

template <typename T, std::size_t N>
Polynomial<T,N> operator+( Polynomial<T,N> p )
{
  return std::move(p);
}

template <typename T, std::size_t N>
Polynomial<T,N> operator-( Polynomial<T,N> p )
{
  for ( std::size_t index = 0; index <= N; ++index )
    p[index] = -p[index];
  return std::move(p);
}

template <typename T, std::size_t M, std::size_t N>
auto operator-( const Polynomial<T,M> & p, const Polynomial<T,N> & q )
{
  return p + (-q);
}

template <typename T, std::size_t N>
Polynomial<T,N> & operator-=( Polynomial<T,N> & p, T val )
{
  p[0] -= val;
  return p;
}

template <typename T, std::size_t N>
auto operator-( Polynomial<T,N> p, T val )
{
  return std::move( p -= val );
}

template <typename T, std::size_t N>
auto operator-( T val, const Polynomial<T,N> & p )
{
  return val + (-p);
}

template <typename T, std::size_t N>
Polynomial<T,N> & operator+=( Polynomial<T,N> & p, T val )
{
  p[0] += val;
  return p;
}

template <typename T, std::size_t N>
auto operator+( Polynomial<T,N> p, T val )
{
  return std::move( p += val );
}

template <typename T, std::size_t N>
auto operator+( T val, Polynomial<T,N> p )
{
  return std::move( p += val );
}


namespace placeholders
{
  template <std::size_t N>
  struct XPower
  {
    template <typename T>
    operator Polynomial<T,N>() const
    {
      Polynomial<T,N> result{};
      result[N] = 1;
      return result;
    }
  };
  XPower<1> X;

  template <std::size_t M, std::size_t N>
  auto operator*( XPower<M>, XPower<N> ) { return XPower<M+N>{}; }

  template <std::size_t N, typename T>
  auto operator*( XPower<N>, T val )
  {
    Polynomial<T,N> result{};
    result[N] = val;
    return result;
  }

  template <typename T, std::size_t N>
  auto operator*( T val, XPower<N> xPower )
  {
    return xPower * val;
  }
} // namespace placeholders

} // namespace cu
