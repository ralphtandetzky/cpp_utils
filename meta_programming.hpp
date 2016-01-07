#pragma once

#include <tuple>
#include <utility>

namespace cu
{

template <typename>
struct tuple_size;

template <typename ...Ts>
struct tuple_size<std::tuple<Ts...> >
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename T> struct tuple_size<const T   > : tuple_size<T> {};
template <typename T> struct tuple_size<volatile T> : tuple_size<T> {};
template <typename T> struct tuple_size<T &       > : tuple_size<T> {};

namespace detail
{

  template <typename Tuple, typename F, std::size_t ...indexes>
  void for_each_impl( Tuple && tuple,
                      F && f, std::index_sequence<indexes...> )
  {
    const auto _ = {(static_cast<void>(std::forward<F>(f)(
        std::get<indexes>(std::forward<Tuple>(tuple)) ) ),1)...};
    static_cast<void>(_);
  }

  template <typename Tuple1, typename Tuple2, typename F, std::size_t ...indexes>
  void for_each_impl( Tuple1 && tuple1,
                      Tuple2 && tuple2,
                      F && f,
                      std::index_sequence<indexes...> )
  {
    const auto _ = { (static_cast<void>( std::forward<F>(f)(
        std::get<indexes>(std::forward<Tuple1>(tuple1)),
        std::get<indexes>(std::forward<Tuple2>(tuple2)) ) ),1)... };
    static_cast<void>(_);
  }

} // namespace detail

template <typename Tuple, typename F>
void for_each( Tuple && tuple, F && f )
{
  detail::for_each_impl(
        tuple,
        std::forward<F>(f),
        std::make_index_sequence<tuple_size<Tuple>::value>() );
}

template <typename Tuple1, typename Tuple2, typename F>
void for_each( Tuple1 && tuple1,
               Tuple2 && tuple2,
               F && f )
{
  static_assert( tuple_size<Tuple1>::value ==
                 tuple_size<Tuple2>::value,
                 "Tuples must have the same length." );
  detail::for_each_impl(
        tuple1,
        tuple2,
        std::forward<F>(f),
        std::make_index_sequence<tuple_size<Tuple1>::value >() );
}


namespace detail
{

  template <typename F>
  struct ReverseAccumulator
  {
    ReverseAccumulator( F && f_ )
      : f(std::forward<F>(f_))
    {}

    template <typename ...Ts, typename T>
    decltype(auto) operator()( T && arg, Ts &&...args ) const
    {
      return f( std::move(*this)( std::forward<Ts>(args)... ),
                std::forward<T>(arg) );
    }

    template <typename T>
    T && operator()( T && arg ) const
    {
      return std::forward<T>(arg);
    }

  private:
    F && f;
  };

  template <typename Tuple, typename F, std::size_t ...indexes>
  decltype(auto) accumulate_impl( Tuple && tuple,
                        F && f,
                        std::index_sequence<indexes...> )
  {
    // The reverse accumulator is used for left to right associativity.
    return ReverseAccumulator<F>(std::forward<F>(f))(
          std::get<tuple_size<Tuple>::value-indexes-1>(std::forward<Tuple>(tuple))... );
  }

} // namespace detail

template <typename Tuple, typename F>
decltype(auto) accumulate( Tuple && tuple,
                 F && f )
{
  return detail::accumulate_impl(
        std::forward<Tuple>(tuple),
        std::forward<F>(f),
        std::make_index_sequence<tuple_size<Tuple>::value>() );
}


namespace detail
{

  template <typename Tuple, typename F, std::size_t ... indexes>
  bool any_of_impl( Tuple && tuple,
                    F && f,
                    std::index_sequence<indexes...> )
  {
    const std::array<bool,sizeof...(indexes)> values =
    { f( std::get<indexes>(tuple) )... };
    for ( bool value : values )
      if ( value )
        return true;
    return false;
  }

} // namespace detail

template <typename Tuple, typename F>
bool any_of( Tuple && tuple,
             F && f )
{
  return detail::any_of_impl(
        std::forward<Tuple>(tuple),
        std::forward<F>(f),
        std::make_index_sequence<tuple_size<Tuple>::value>() );
}

template <std::size_t N>
class Rank;

template <>
class Rank<0> {};

template <std::size_t N>
class Rank : public Rank<N-1> {};

} // namespace cu
