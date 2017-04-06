// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include <algorithm>
#include <cassert>

namespace cu
{

/// One-dimesional clipping. A value is clipped to an interval.
///
/// If the value @c toBeClipped lies within the interval
/// @c [lowerBound,upperBound], then The value will be returned unchanged.
/// Otherwise one of the end points of the interval will be returned,
/// whichever is closer.
template <typename T>
T clipped( const T & toBeClipped, const T & lowerBound, const T & upperBound )
{
  assert( lowerBound <= upperBound );
  return std::max( lowerBound, std::min( upperBound, toBeClipped ) );
}


/// Calculates the square of the passed parameter.
template <typename T>
constexpr auto sqr( T && x )
{
  return x*x;
}


/// Calculates the sign of a value.
///
/// Works for both integers and floating point types.
template <typename T>
int sgn( T val ) {
    return ( T(0) < val ) - ( val < T(0) );
}

} // namespace cu
