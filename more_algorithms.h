/// @file
///
/// @author Ralph Tandetzky
/// @date 17 Aug 2013

#pragma once

#include <algorithm>
#include <cassert>
#include <numeric>

namespace cu {

/// \brief Calculates the square of a value.
template <typename T>
auto sqr( const T & t ) -> decltype(t*t)
{
    return t*t;
}


/// Iterator checked implementation of @c std::for_each().
template <typename InputIterator1, typename InputIterator2, typename BinOp>
BinOp for_each( InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, InputIterator2 last2,
                BinOp op )
{
    while ( first1 != last1 )
    {
        assert( first2 != last2 );
        op( *first1, *first2 );
        ++first1;
        ++first2;
    }
    assert( first2 == last2 );
    return std::move(op);
}


/// \brief Optimized @c std::accumulate() which uses move semantics.
///
/// The @c init argument is moved into the binary operation.
template <class InputIterator, class T, class BinOp>
T movingAccumulate (
        InputIterator first, InputIterator last, T init, BinOp binop)
{
    while ( first != last ) {
        init = binop( std::move(init), *first );
        ++first;
    }
    return init;
}


/// \brief Optimized @c std::accumulate() which uses move semantics.
///
/// The @c init argument is moved into the binary @c operator+().
template <class InputIterator, class T, class BinOp>
T movingAccumulate (
        InputIterator first, InputIterator last, T init )
{
    while ( first != last ) {
        init = std::move(init) + *first;
        ++first;
    }
    return init;
}


/// \brief Calculates range1 += factor * range2.
template <typename T, typename InputIterator1, typename InputIterator2>
void addAssign( InputIterator1 first1, InputIterator1 last1, T factor,
                InputIterator2 first2, InputIterator2 last2 )
{
    if ( factor == 1 )
        for_each( first1, last1, first2, last2,
                  []( T & x, const T & y ){ x += y; } );
    else if ( factor == -1 )
        for_each( first1, last1, first2, last2,
                  []( T & x, const T & y ){ x -= y; } );
    else
        for_each( first1, last1, first2, last2,
                  [factor]( T & x, const T & y ){ x += factor * y; } );
}


/// \brief Calculates range1 -= factor * range2.
template <typename T, typename InputIterator1, typename InputIterator2>
void subAssign( InputIterator1 first1, InputIterator1 last1, T factor,
                InputIterator2 first2, InputIterator2 last2 )
{
    return addAssign( first1, last1, -factor, first2, last2 );
}


/// Multiplies all elements of a range with a factor.
template <typename T, typename InputIterator>
void mulAssign( InputIterator first, InputIterator last, T factor )
{
    std::for_each( first, last, [factor]( T & x ){ x*=factor; } );
}


/// Divides all elements of a range by a constant.
template <typename T, typename InputIterator>
void divAssign( InputIterator first, InputIterator last, T t )
{
    return mulAssign( first, last, 1/t );
}


/// \brief Calculated the inner product of two ranges.
///
/// \pre The ranges must be equally long.
template <typename T, typename InputIterator1, typename InputIterator2>
T innerProduct( InputIterator1 first1, InputIterator1 last1,
                InputIterator2 first2, InputIterator2 last2,
                T startValue )
{
    while ( first1 != last1 )
    {
        assert( first2 != last2 );
        startValue += *first1 * *first2;
        ++first1;
        ++first2;
    }
    assert( first2 == last2 );
    return startValue;
}


/// \brief Calculates the l_2 norm of a range.
template <typename T, typename InputIterator>
T squareNorm( InputIterator first, InputIterator last, T startValue )
{
    while ( first != last )
    {
        startValue += sqr(*first);
        ++first;
    }

    return startValue;
}


/// \brief Returns an orthonormal base that spans the same space as
/// the vectors in the input matrix, if the input vectors are linearly
/// independent.
///
/// If the input vectors are not linearly independent, then some vectors
/// of the result matrix will be zero. The same space will be spanned then,
/// but the vectors are not normed.
///
/// It is asserted that the length of all input vectors is the same.
/// It is also asserted that the number of vectors is at most the length
/// of the given vectors.
template <typename T>
std::vector<std::vector<T> > gramSchmidtProcess(
        std::vector<std::vector<T>> matrix )
{
    if ( matrix.empty() )
        return matrix;

    const auto m = matrix.size();
    const auto n = matrix.front().size();

    assert( m <= n );

    for ( auto i = size_t{0}; i < m; ++i )
    {
        auto & mi = matrix[i];
        assert( mi.size() == n );
        for ( auto j = size_t{0}; j < i; ++j )
        {
            const auto & mj = matrix[j];
            const auto prod = cu::innerProduct(
                    begin(mi), end(mi),
                    begin(mj), end(mj), T{} );
            subAssign( begin(mi), end(mi), prod,
                       begin(mj), end(mj) );
        }
        const auto norm2 = cu::squareNorm( begin(mi), end(mi), T{} );
        if ( norm2 == T{} )
            continue;
        cu::divAssign( begin(mi), end(mi), sqrt(norm2) );
    }
    return matrix;
}


/// Like @c std::swap, but guaranteed to never fail.
template <typename T>
void nofail_swap( T & lhs, T & rhs )
{
    using namespace std;
    static_assert( noexcept( swap(lhs, rhs) ),
                   "This swap operation must not throw." );
    swap(lhs,rhs);
}


/// \brief Removes certain elements from a beginning and end of a container.
///
/// Typically, an @c std::string is trimmed of whitespace characters. For
/// this purpose the overload taking an std::string is especially provided.
template <typename Container, typename F>
Container trim( const Container & s, F && shallBeTrimmed )
{
    auto first = begin(s);
    auto last = end(s);

    while ( first != last && shallBeTrimmed(*first) )
        ++first;
    while ( last != first && shallBeTrimmed(*std::prev(last)) )
        --last; // NOOP

    return std::string( first, last );
}

inline std::string trim( const std::string & s )
{
    return trim( s, (int(*)(int))std::isspace );
}


/// \brief Calculated the editing distance of two strings.
///
/// Returns the number of steps that are necessary to transform the string
/// @c s into @c t by inserting, deleting and replacing single characters.
inline size_t levenshteinDistance( const std::string & s,
                                   const std::string & t )
{
    if ( s == t )    return 0;
    if ( s.empty() ) return t.size();
    if ( t.empty() ) return s.size();

    auto v0 = std::vector<size_t>(t.size()+1);
    auto v1 = v0;
    std::iota( begin(v0), end(v0), 0 );
    for ( auto i = size_t{0}; i < s.size(); ++i)
    {
        v1[0] = i + 1;

        for ( auto j = size_t{0}; j < t.size(); ++j)
        {
            const auto cost = s[i] == t[j] ? 0 : 1;
            v1[j + 1] = std::min( std::min( v1[j], v0[j + 1] ) + 1,
                    v0[j] + cost );
        }
        v0.swap( v1 );
    }

    return v0[t.size()];
}


template <typename ForwardIt1, typename ForwardIt2>
ForwardIt2 findBoyerMoore( ForwardIt1 firstNeedle,
                           ForwardIt1 lastNeedle,
                           ForwardIt2 firstHaystack,
                           ForwardIt2 lastHaystack )
{
    while ( firstHaystack != lastHaystack )
    {
        auto it1 = firstNeedle;
        auto it2 = firstHaystack;
        while ( *it1 == *it2 && it1 != lastNeedle && it2 != lastHaystack  )
        {
            ++it1;
            ++it2;
        }
        if ( it1 == lastNeedle )
            return firstHaystack;
        ++firstHaystack;
    }
    return lastHaystack;
}

} // namespace cu
