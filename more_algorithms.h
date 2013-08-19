#pragma once

namespace cu {

template <class InputIterator, class T, class BinOp>
T moving_accumulate (
        InputIterator first, InputIterator last, T init, BinOp binop)
{
  while ( first != last ) {
    init = binop( std::move(init), *first );
    ++first;
  }
  return init;
}


template <class InputIterator, class T, class BinOp>
T moving_accumulate (
        InputIterator first, InputIterator last, T init )
{
  while ( first != last ) {
    init = std::move(init) + *first;
    ++first;
  }
  return init;
}

} // namespace cu
