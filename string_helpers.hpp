// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include <algorithm>
#include <cassert>
#include <istream>
#include <string>

namespace cu
{


/// Tells whether the @c txt string starts with the @c start string.
template <std::size_t N>
bool startsWith( const std::string & txt, const char (&start)[N] )
{
  // The actual length of the `start` string is N-1, because of the
  // terminating '\0' character.
  const auto startLength = N-1;
  if ( txt.size() < startLength )
    return false;
  return std::equal( start, start + startLength,
                     txt.begin() );
}


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
                     txt.end() - static_cast<std::ptrdiff_t>( end.size() ) );
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


inline std::string replaceEndWith(
    std::string input,
    const std::string & newEnd )
{
  assert( newEnd.size() <= input.size() );
  auto newEndStart = input.end()-static_cast<std::ptrdiff_t>(newEnd.size());
  while ( (*newEndStart & 0xC0 ) == 0x80 ) // care about unicode (UTF-8)
    --newEndStart;
  std::copy( newEnd.begin(),
             newEnd.end  (),
             newEndStart );
  input.resize( static_cast<std::size_t>(
             newEndStart + static_cast<std::ptrdiff_t>(newEnd.size()) - input.begin() ) );
  return input;
}


inline std::string getLine( std::istream & stream )
{
  std::string line;
  std::getline( stream, line );
  return line;
}


inline std::string getLine( std::istream && stream )
{
  return getLine( stream );
}

} // namespace cu
