#pragma once

#include <algorithm>
#include <string>

namespace cu
{


/// Tells whether the @c txt string ends with the @c end string.
template <std::size_t N>
bool endsWith( const std::string & txt, const char (&end)[N] )
{
  // The actual length of the `end` string is N-1, because of the
  // terminating '\0' character.
  const auto endLength = N-1;
  if ( txt.size() < endLength )
    return false;
  return std::equal( end, end + endLength,
                     txt.end()- endLength );
}


inline bool endsWith( const std::string & txt, const std::string & end )
{
  if ( txt.size() < end.size() )
    return false;
  return std::equal( end.begin(), end.end(),
                     txt.end() - end.size() );
}


} // namespace cu
