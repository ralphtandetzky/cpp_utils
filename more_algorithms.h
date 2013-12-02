/// @file
///
/// @author Ralph Tandetzky
/// @date 17 Aug 2013

#pragma once

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
BinOp for_each( InputIterator1 first1, InputIterator2 last1,
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

} // namespace cu
