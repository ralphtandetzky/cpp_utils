/** @file Define the template function ignore().
 * @author Ralph Tandetzky
 */

#pragma once

namespace cu
{

/// To avoid 'unused parameter' warnings pass the parameters to this function.
///
/// @example Here's how it's used:
/// @code
///   void f( int i, int j )
///   {
///     assert( i != 0 && j != 0 );
///     cu::ignore( i, j );
///   }
/// @endcode
template <typename ...T>
void ignore( const T &... ) {}

} // namespace cu
