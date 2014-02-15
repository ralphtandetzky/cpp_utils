/// @file
///
/// @author Ralph Tandetzky
/// @date 17 Aug 2013

#pragma once

#include <cassert>

namespace cu {

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

} // namespace cu
