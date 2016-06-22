#pragma once

#include <array>
#include <tuple>
#include <utility>

#define CU_FWD(arg) ::std::forward<decltype(arg)>(arg)

namespace cu
{

/// Returns the size of a tuple as compile-time constant.
template <typename ...Ts>
constexpr std::size_t get_tuple_size( const std::tuple<Ts...> & )
{
  return sizeof...(Ts);
}

template <typename Tuple>
constexpr auto make_index_sequence( const Tuple & t )
{
  return std::make_index_sequence<get_tuple_size(t)>();
}


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

/// Applies a functor to each element of a tuple.
///
/// The types of the elements can differ and it still works.
template <typename Tuple, typename F>
void for_each( Tuple && tuple, F && f )
{
  detail::for_each_impl(
        std::forward<Tuple>(tuple),
        std::forward<F>(f),
        make_index_sequence(tuple) );
}

/// Applies a functor to the elements of two tuples.
///
/// In effect, the following is called:
///   @code
///     f( get< 0 >(std::forward<Tuple1>(tuple1)),
///        get< 0 >(std::forward<Tuple2>(tuple2)) );
///         .
///         .
///         .
///     f( get<N-1>(std::forward<Tuple1>(tuple1)),
///        get<N-1>(std::forward<Tuple2>(tuple2)) );
///   @endcode
/// where @c N ist the size of both tuples. Note also, that proper forwarding
/// is applied to the elements of the tuples.
template <typename Tuple1, typename Tuple2, typename F>
void for_each( Tuple1 && tuple1,
               Tuple2 && tuple2,
               F && f )
{
  static_assert( get_tuple_size(tuple1) ==
                 get_tuple_size(tuple2),
                 "Tuples must have the same length." );
  detail::for_each_impl(
        std::forward<Tuple1>(tuple1),
        std::forward<Tuple2>(tuple2),
        std::forward<F>(f),
        make_index_sequence(tuple1) );
}


namespace detail
{

  template <typename Tuple, typename F, std::size_t ...indexes>
  decltype(auto) transform_impl( Tuple && tuple,
                                 F && f,
                                 std::index_sequence<indexes...> )
  {
    return std::make_tuple( f( std::get<indexes>( std::forward<Tuple>(tuple) ) )... );
  }

} // namespace detail

/// A functor is applied to every element of a tuple and the returned values
/// are returned in a tuple.
template <typename Tuple, typename F>
decltype(auto) transform( Tuple && tuple,
                          F && f )
{
  return detail::transform_impl(
        std::forward<Tuple>(tuple),
        std::forward<F>(f),
        make_index_sequence(tuple) );
}


namespace detail
{

  template <typename T, typename Tuple, std::size_t ...indexes>
  auto to_array_impl1( Tuple && tuple, std::index_sequence<indexes...> )
  {
    return std::array<T,get_tuple_size(tuple)>{
      std::get<indexes>( std::forward<Tuple>(tuple))... };
  }

}

/// Turns a tuple into an array.
///
/// The element type is not inferred, but must be specified.
template <typename T, typename Tuple>
auto to_array( Tuple && tuple )
{
  return detail::to_array_impl1<T>(
        std::forward<Tuple>(tuple),
        make_index_sequence(tuple) );
}


namespace detail
{

  template <typename T, std::size_t ...indexes>
  auto to_array_impl2( T * arr, std::index_sequence<indexes...> )
  {
    return std::array<std::decay_t<T>,sizeof...(indexes)>{ {
          arr[indexes]...
          } };
  }

} // namespace detail

template <typename T, std::size_t N>
auto to_array( T(&arr)[N] )
{
  return detail::to_array_impl2( arr, std::make_index_sequence<N>() );
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
    // It must be reverse because elements of an argument pack must be
    // removed from the front while inferring parameters. In other words,
    // a template of the form
    //   @code
    //     template <typename ...Ts, typename T>
    //     void f( Ts &&...args, T && arg );
    //   @endcode
    // is ill-formed code, while
    //   @code
    //     template <typename ...Ts, typename T>
    //     void f( T && arg, Ts &&...args );
    //   @endcode
    // is perfectly valid.
    return ReverseAccumulator<F>(std::forward<F>(f))(
          std::get<get_tuple_size(tuple)-indexes-1>(std::forward<Tuple>(tuple))... );
  }

} // namespace detail

/// Accumulates the elements of a tuple given a specific functor.
///
/// If the elements of the tuple shall be added together, then the functor
/// parameter should be something like this:
///   @code
///     []( auto && rhs, auto && lhs ) { return rhs + lhs; }
///   @endcode
/// Note that the operation will be performed left to right associative
/// independent of the type of functor used.
template <typename Tuple, typename F>
decltype(auto) accumulate( Tuple && tuple,
                 F && f )
{
  return detail::accumulate_impl(
        std::forward<Tuple>(tuple),
        std::forward<F>(f),
        make_index_sequence(tuple) );
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

/// Returns @c true, iff any of the @c tuple elements evaluates to @true.
template <typename Tuple, typename F>
bool any_of( Tuple && tuple,
             F && f )
{
  return detail::any_of_impl(
        std::forward<Tuple>(tuple),
        std::forward<F>(f),
        make_index_sequence(tuple) );
}

/// This is a helper class which enables the prioritization of overloads.
///
/// If there are two overloads of a function that only differ in one argument
/// type, which are @c Rank<N> and @c Rang<M>, and the function is called
/// given an argument of type @c Rank<K> where @c K>=N and @K>=M, then
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
