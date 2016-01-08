#pragma once

#include <algorithm>
#include <cassert>

namespace cu
{

template <typename T>
T clipped( const T & toBeClipped, const T & lowerBound, const T & upperBound )
{
  assert( lowerBound <= upperBound );
  return std::max( lowerBound, std::min( upperBound, toBeClipped ) );
}

} // namespace cu
