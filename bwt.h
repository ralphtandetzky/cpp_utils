/// \file 
/// \brief This file provides functions around the Burrows-Wheeler-Transform.

#pragma once

#include <string>

namespace cu {

/// \brief Performes a Burrows-Wheeler transform on the input string.
///
/// The first part of the returned value is the string the standard transform
/// described on http://en.wikipedia.org/wiki/Burrows%E2%80%93Wheeler_transform
/// would return. The second part of the returned value tells which rotation
/// to pick when applying the inverse transform. When calling 
///   @code
///     burrows_wheeler_transform_inverse( r.first, r.second )
///   @endcode
/// on the returned value \c r, then the original string will be returned. 
/// Without \c r.second the original string could only be restored modulo
/// a rotation of the string. If all rotations of the string would be sorted
/// alphabetically, then the resulting first string must be rotated to the 
/// left by \c r.second positions in order to obtain the original string.
std::pair<std::string,size_t> burrows_wheeler_transform( std::string s );

/// \brief Performes an inverse Burrows-Wheeler transform.
///
/// This function satisfies the following assertion.
///   @code
///     const std::string s = ...
///     auto r = burrows_wheeler_transform( s );
///     assert( burrows_wheeler_transform_inverse( r.first, r.second ) == s );
///   @endcode
/// @see burrows_wheeler_transform
std::string burrows_wheeler_transform_inverse( std::string s, size_t index );

} // namespace cu
