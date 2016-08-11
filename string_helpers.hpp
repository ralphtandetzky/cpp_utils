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


/// Tells whether the @c txt string ends with the @c end string.
inline bool endsWith( const std::string & txt, const std::string & end )
{
  if ( txt.size() < end.size() )
    return false;
  return std::equal( end.begin(), end.end(),
                     txt.end() - end.size() );
}


/// Removes whitespaces from the beginning and end of a string.
inline std::string trim( const std::string & s )
{
  auto first = s.begin();
  auto last = s.end();
  while ( first < last && std::isspace( *first) )
    ++first;
  while ( last > first && std::isspace( *std::prev(last) ) )
    --last; // NOOP

  return { first, last };
}


} // namespace cu
