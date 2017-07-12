#pragma once

#include <cassert>
#include <vector>

namespace cu
{
namespace vector_arith
{

namespace detail
{

    template <typename View>
    struct Expression : View
    {
        using View::View;
        using View::operator[];
        using View::size;
    };

    template <typename T>
    struct WrapperView
    {
        WrapperView( const T & wrapped_ )
            : wrapped(wrapped_)
        {
        }

        WrapperView & operator=(WrapperView) = delete;

        decltype(auto) operator[]( std::size_t index ) const
        {
            return wrapped[index];
        }

        std::size_t size() const
        {
            return wrapped.size();
        }

        const T & wrapped;
    };

    template <typename T>
    Expression<WrapperView<T>> makeWrapperView( const T & wrapped )
    {
        return { wrapped };
    }

    template <typename T1,
              typename T2,
              typename F>
    struct CombinedView
    {
        CombinedView( T1 lhs_, T2 rhs_, F f_ )
            : lhs(lhs_)
            , rhs(rhs_)
            , f(std::move(f_))
        {
        }

        decltype(auto) operator[]( std::size_t index ) const
        {
            return f( lhs[index], rhs[index] );
        }

        std::size_t size() const
        {
            return lhs.size();
        }

        const T1 lhs;
        const T2 rhs;
        F f;
    };

    template <typename T1,
              typename T2,
              typename F>
    Expression<CombinedView<T1,T2,F>> makeCombinedView(
            T1 lhs, T2 rhs, F f )
    {
        return { lhs, rhs, f };
    }

    template <typename View1,
              typename View2>
    auto operator+( Expression<View1> lhs, Expression<View2> rhs )
    {
        return detail::makeCombinedView( lhs, rhs,
            []( const auto & lhs, const auto & rhs ) { return lhs+rhs; } );
    }

    template <typename View,
              typename T>
    auto operator+( Expression<View> lhs, const std::vector<T> & rhs )
    {
        return lhs + makeWrapperView( rhs );
    }

    template <typename T,
              typename View>
    auto operator+( const std::vector<T> & lhs, const Expression<View> & rhs )
    {
        return makeWrapperView( lhs ) + rhs;
    }

    template <typename View1,
              typename View2>
    auto operator-( const Expression<View1> & lhs, const Expression<View2> & rhs )
    {
        return detail::makeCombinedView( lhs, rhs,
            []( const auto & lhs, const auto & rhs ) { return lhs-rhs; } );
    }

    template <typename View,
              typename T>
    auto operator-( const Expression<View> & lhs, const std::vector<T> & rhs )
    {
        return lhs - makeWrapperView( rhs );
    }

    template <typename T,
              typename View>
    auto operator-( const std::vector<T> & lhs, const Expression<View> & rhs )
    {
        return makeWrapperView( lhs ) - rhs;
    }

} // namespace detail

template <typename T1,
          typename T2>
auto operator+( const std::vector<T1> & lhs, const std::vector<T2> & rhs )
{
    return detail::makeWrapperView( lhs ) + detail::makeWrapperView( rhs );
}

template <typename T1,
          typename T2>
auto operator-( const std::vector<T1> & lhs, const std::vector<T2> & rhs )
{
    return detail::makeWrapperView( lhs ) - detail::makeWrapperView( rhs );
}

template <typename T,
          typename F>
void assign( std::vector<T> & v, detail::Expression<F> && expr )
{
    const auto size = expr.size();
    v.clear();
    v.reserve(size);
    for ( std::size_t i = 0; i < size; ++i )
        v.push_back( expr[i] );
}

} // namespace vector_arith
} // namespace cu
