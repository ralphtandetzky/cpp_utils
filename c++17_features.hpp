/** @file This file defines features available from C++17 onwards.
 * The features are defined in the cu namespace instead of the std namespace.
 * When moving to C++17 just remove this header from the project,
 * in order to keep you project clean. This should require only a bit of
 * refactoring by replacing the respective @c cu:: by @c std::.
 *
 * @author Ralph Tandetzky
 */

#pragma once

#include <cstddef>

namespace cu
{

template <class Container>
constexpr auto size( const Container& container ) -> decltype(container.size())
{
    return container.size();
}


template <class T, std::size_t N>
constexpr std::size_t size( const T (&)[N] ) noexcept
{
    return N;
}

} // namespace cu
