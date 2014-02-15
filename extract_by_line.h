/// @file
///
/// @author Ralph Tandetzky
/// @date 15 Feb 2014

#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace cu
{

/// \brief Extracts a stream line by line.
///
/// Every line will be stored as an element of the output vector.
/// The stream will be extracted until @c is.eof() is true. On
/// failure an exception is thrown and the stream is left in an
/// undefined but valid state (basic exception guarantee).
std::vector<std::string> extractByLine( std::istream &  is );
std::vector<std::string> extractByLine( std::istream && is );

} // namespace cu
