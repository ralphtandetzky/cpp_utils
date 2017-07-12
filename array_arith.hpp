/** This header defines simple arithmetics for @c std::array<T,N> treating
  * them as vectors of fixed size. The following operators are supported:
  *   - operator+( const std::array<T,N> &, const std::array<T,N> & )
  *   - operator-( const std::array<T,N> &, const std::array<T,N> & )
  *   - operator*( const T2 &, const std::array<T1,N> & )
  *   - operator/( const std::array<T1,N> &, const T2 & )
  * To enable these operators conveniently write
  *   @code
  *     using namespace cu::array_arith;
  *   @endcode
  * into your code.
*/

#pragma once

#include <array>
#include <utility>

namespace cu
{

namespace array_arith
{

namespace detail
{
    template <typename T,
              std::size_t N,
              std::size_t ...idxs>
    std::array<T,N> add_impl( const std::array<T,N> & lhs,
                              const std::array<T,N> & rhs,
                              std::index_sequence<idxs...> )
    {
        return { lhs[idxs]+rhs[idxs]... };
    }
} // namespace detail

template <typename T,
          std::size_t N>
std::array<T,N> operator+( const std::array<T,N> & lhs,
                           const std::array<T,N> & rhs )
{
    return detail::add_impl( lhs, rhs, std::make_index_sequence<N>{} );
}

namespace detail
{
    template <typename T,
              std::size_t N,
              std::size_t ...idxs>
    std::array<T,N> sub_impl( const std::array<T,N> & lhs,
                              const std::array<T,N> & rhs,
                              std::index_sequence<idxs...> )
    {
        return { lhs[idxs]-rhs[idxs]... };
    }
} // namespace detail

template <typename T,
          std::size_t N>
std::array<T,N> operator-( const std::array<T,N> & lhs,
                           const std::array<T,N> & rhs )
{
    return detail::sub_impl( lhs, rhs, std::make_index_sequence<N>{} );
}

namespace detail
{
    template <typename T1,
              typename T2,
              std::size_t N,
              std::size_t ...idxs>
    std::array<T1,N> mul_impl( const T2 & lhs,
                               const std::array<T1,N> & rhs,
                               std::index_sequence<idxs...> )
    {
        return { lhs*rhs[idxs]... };
    }
} // namespace detail

template <typename T1,
          typename T2,
          std::size_t N>
std::array<T1,N> operator*( const T2 & lhs,
                            const std::array<T1,N> & rhs )
{
    return detail::mul_impl( lhs, rhs, std::make_index_sequence<N>{} );
}

namespace detail
{
    template <typename T1,
              typename T2,
              std::size_t N,
              std::size_t ...idxs>
    std::array<T1,N> div_impl( const std::array<T1,N> & lhs,
                               const T2 & rhs,
                               std::index_sequence<idxs...> )
    {
        const auto reciprocal = T1(1) / rhs;
        return { lhs[idxs]*reciprocal... };
    }
} // namespace detail

template <typename T1,
          typename T2,
          std::size_t N>
std::array<T1,N> operator/( const std::array<T1,N> & lhs,
                            const T2 & rhs )
{
    return detail::div_impl( lhs, rhs, std::make_index_sequence<N>{} );
}

template <typename T,
          std::size_t N>
void assign( std::array<T,N> & lhs, std::array<T,N> && rhs )
{
    lhs = std::move(rhs);
}

} // namespace array_arith

} // namespace cu
