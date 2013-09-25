/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

namespace cu {

template <typename T>
auto sqr( const T & t ) -> decltype(t*t)
{
    return t*t;
}

} // namespace cu
