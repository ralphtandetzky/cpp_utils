// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include <cstddef>

namespace cu
{

/// This is a helper class which enables the prioritization of overloads.
///
/// If there are two overloads of a function that only differ in one argument
/// type, which are @c Rank<N> and @c Rang<M>, and the function is called
/// given an argument of type @c Rank<K> where @c K>=N and @c K>=M, then
/// the overload with the larger @c Rank number will be selected by the
/// compiler.
///
/// This is helpful, when an overload shall be prioritized over another and
/// the prioritized overload may be excluded from overload resolution because
/// of SFINAE (Substitution Failure Is Not An Error).
template <std::size_t N>
class Rank;

template <>
class Rank<0> {};

template <std::size_t N>
class Rank : public Rank<N-1> {};

} // namespace cu
