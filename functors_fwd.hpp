/** @file Forward declares the template classes @c Lambda and @c MoveFunction.
 *
 * Prefer to include this header file rather than "functors.hpp" in
 * header files. This reduces compile-time dependecies.
 *
 * @author Ralph Tandetzky
 */

#pragma once

namespace cu
{

template <typename Signature>
class Lambda;

template <typename Signature>
class MoveFunction;

} // namespace cu
