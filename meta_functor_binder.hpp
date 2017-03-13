// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include "meta_programming.hpp"

#include <functional>
#include <utility>

namespace cu
{

template <typename T, typename MetaFunctor>
class MetaFunctorArgumentBinder
{
  std::pair<T,MetaFunctor> data;

public:
  template <typename TArgsTuple, typename MetaFunctorArgsTuple>
  MetaFunctorArgumentBinder(
      TArgsTuple && tArgs,
      MetaFunctorArgsTuple && metaFunctorArgs )
    : data( std::piecewise_construct,
            CU_FWD(tArgs),
            CU_FWD(metaFunctorArgs) )
  {}

  template <typename F>
  decltype(auto) operator()( F && f )
  {
    return data.second( std::bind( std::forward<F>(f),
                                   std::ref(data.first) ) );
  }

  template <typename F>
  decltype(auto) operator()( F && f ) const
  {
    return data.second( std::bind( std::forward<F>(f),
                                   std::ref(data.first) ) );
  }

};

} // namespace cu
