/// @file
///
/// @author Ralph Tandetzky
/// @date 17 Aug 2013

#pragma once

#include <cassert>
#include <numeric>

namespace cu {

/// \brief Optimized @c std::accumulate() which uses move semantics.
///
/// The @c init argument is moved into the binary operation.
template <class InputIterator, class T, class BinOp>
T MovingAccumulate (
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
T MovingAccumulate (
        InputIterator first, InputIterator last, T init )
{
    while ( first != last ) {
        init = std::move(init) + *first;
        ++first;
    }
    return init;
}


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

} // namespace cu
