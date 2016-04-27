#pragma once

#include "meta_programming.hpp"

namespace cu
{

template <typename T, typename MetaFunctor>
class MetaFunctorArgumentBinder
{
  T item;
  MetaFunctor metaFunctor;

  template <typename ...TArgs,
            std::size_t ...tArgsIndices,
            typename ...MetaFunctorArgs,
            std::size_t ...metaFunctorArgsIndices>
  MetaFunctorArgumentBinder(
      const std::tuple<TArgs...> & tArgs,
      std::index_sequence<tArgsIndices...>,
      const std::tuple<MetaFunctorArgs...> & metaFunctorArgs,
      std::index_sequence<metaFunctorArgsIndices...> )
    : item       ( std::forward<TArgs          >(std::get<tArgsIndices          >(tArgs          ))... )
    , metaFunctor( std::forward<MetaFunctorArgs>(std::get<metaFunctorArgsIndices>(metaFunctorArgs))... )
  {
  }

public:
  template <typename TArgsTuple, typename MetaFunctorArgsTuple>
  MetaFunctorArgumentBinder(
      TArgsTuple && tArgs,
      MetaFunctorArgsTuple && metaFunctorArgs )
    : MetaFunctorArgumentBinder(
        std::forward<TArgsTuple>(tArgs),
        make_index_sequence(tArgs),
        std::forward<MetaFunctorArgsTuple>(metaFunctorArgs),
        make_index_sequence(metaFunctorArgs)
        )
  {}

  template <typename F>
  decltype(auto) operator()( F && f )
  {
    return metaFunctor( std::bind( std::forward<F>(f), std::ref(item) ) );
  }

  template <typename F>
  decltype(auto) operator()( F && f ) const
  {
    return metaFunctor( std::bind( std::forward<F>(f), std::ref(item) ) );
  }

};

} // namespace cu
