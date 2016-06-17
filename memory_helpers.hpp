#pragma once

#include <memory>

namespace cu
{

template <typename T>
std::shared_ptr<std::decay_t<T>> to_shared_ptr( T && x )
{
  return std::make_shared<T>( std::forward<T>(x) );
}

} // namespace cu
