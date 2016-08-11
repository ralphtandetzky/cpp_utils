#pragma once

namespace cu
{

/// Search for the first element in a range satisfying a certain condition
/// using a binary search.
///
/// @pre If the predicate is satisfied for an element in the range, then
///   the predicate must be satisfied for all elements after it.
template <typename RandIt,
          typename Pred>
RandIt find_first_binary_search(
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

} // namespace cu
