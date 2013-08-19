#pragma once

namespace cu {

template <typename T>
auto sqr( const T & t ) -> decltype(t*t)
{
    return t*t;
}

} // namespace cu
