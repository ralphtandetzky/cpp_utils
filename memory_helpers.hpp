#pragma once

#include <memory>
#include <type_traits>

namespace cu
{

template <typename T>
std::unique_ptr<std::decay_t<T>> to_unique_ptr( T && x )
{
  return std::make_unique<std::decay_t<T>>( std::forward<T>(x) );
}

template <typename T>
std::shared_ptr<std::decay_t<T>> to_shared_ptr( T && x )
{
  return std::make_shared<std::decay_t<T>>( std::forward<T>(x) );
}

} // namespace cu
