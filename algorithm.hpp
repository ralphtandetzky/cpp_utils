#pragma once

#include <utility>

namespace cu
{

/// Search for the first element in a range satisfying a certain condition
/// using a binary search.
///
/// @pre If the predicate is satisfied for an element in the range, then
///   the predicate must be satisfied for all elements after it.
template <typename RandIt,
          typename Pred>
constexpr RandIt find_first_binary_search(
    RandIt first, RandIt last, Pred && pred )
{
  auto dist = last - first;

  if ( dist == 0 || pred( *first ) )
    return first;

  // invariant: !pred( *first ) && last >= returned value
  // in each iteration the distance between first and last is reduced by
  // at least their distance divided by 2.
  while ( dist > 1 )
  {
    const auto mid = last - dist/2;
    if ( pred( *mid ) )
    {
      last = mid;
      dist -= dist/2;
    }
    else
    {
      first = mid;
      dist /= 2;
    }
  }

  return last;
}


/// Search for the first element in a range satisfying a certain condition
/// using a hint-based binary search.
///
/// @pre If the predicate is satisfied for an element in the range, then
///   the predicate must be satisfied for all elements after it.
///
/// @param hint The place where the search starts.
template <typename RandIt,
          typename Pred>
constexpr RandIt find_first_with_hint_binary_search(
    RandIt first, RandIt last, RandIt hint, Pred && pred )
{
  if ( first == last )
    return first;

  if ( hint == last )
    --hint;

  std::ptrdiff_t inc = 1;
  if ( pred(*hint) )
  {
    for (;;)
    {
      const auto s = hint - inc;
      if ( s <= first )
        return find_first_binary_search( first, hint, std::forward<Pred>(pred) );
      if ( !pred(*s) )
        return find_first_binary_search( s+1, hint, std::forward<Pred>(pred) );
      hint = s;
      inc *= 2;
    }
  }

  for (;;)
  {
    const auto s = hint + inc;
    if ( s >= last )
      return find_first_binary_search( hint+1, last, std::forward<Pred>(pred) );
    if ( pred(*s) )
      return find_first_binary_search( hint+1, s, std::forward<Pred>(pred) );
    hint = s;
    inc *= 2;
  }
}

} // namespace cu
