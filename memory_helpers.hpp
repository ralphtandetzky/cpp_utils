// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include "rank.hpp"

#include <memory>
#include <type_traits>

namespace cu
{

template <typename T>
std::unique_ptr<std::decay_t<T>> to_unique_ptr( T && x )
{
  return std::make_unique<std::decay_t<T>>( std::forward<T>(x) );
}

namespace detail
{
  template <typename T>
  struct is_unique_ptr : std::false_type {};

  template <typename ...Ts>
  struct is_unique_ptr<std::unique_ptr<Ts...> > : std::true_type {};

  template <typename T>
  auto to_shared_ptr_impl( T && x, Rank<1> )
    -> typename
      std::enable_if<
        is_unique_ptr<typename std::decay<T>::type>::value,
        std::shared_ptr<
          typename std::decay<decltype(x)>::type
        >
      >::type
  {
    return { std::forward<T>(x) };
  }

  template <typename T>
  std::shared_ptr<std::decay_t<T>> to_shared_ptr_impl( T && x, Rank<0> )
  {
    return std::make_shared<std::decay_t<T>>( std::forward<T>(x) );
  }

} // namespace detail

template <typename T>
std::shared_ptr<std::decay_t<T>> to_shared_ptr( T && x )
{
  return detail::to_shared_ptr_impl( std::forward<T>(x), Rank<1>() );
}

} // namespace cu
