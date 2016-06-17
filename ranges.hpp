#pragma once

#include "c++17_features.hpp"
#include "meta_programming.hpp"

#include <cassert>

namespace cu
{

/**********************
 * Generic put method *
 **********************/

namespace detail
{

  template <typename OutputRange,
            typename Value>
  auto put_impl( OutputRange && out, Value && val, cu::Rank<1> )
    -> decltype(out.put(std::forward<Value>(val)))
  {
    return out.put(std::forward<Value>(val));
  }

  template <typename OutputRange,
            typename Value>
  auto put_impl( OutputRange && out, Value && val, cu::Rank<0> )
    -> decltype(static_cast<void>(out.front()=std::forward<Value>(val)),
                out.pop_front())
  {
    return static_cast<void>(out.front()=std::forward<Value>(val)),
           out.pop_front();
  }

} // namespace detail

template <typename OutputRange,
          typename Value>
decltype(auto) put( OutputRange out, Value && val )
{
  return detail::put_impl( std::move(out), std::forward<Value>(val), cu::Rank<1>() );
}


/***************************
 * Range factory functions *
 ***************************/

namespace detail
{

  template <typename Iter1,
            typename Iter2>
  class IteratorRange
  {
  public:
    IteratorRange( Iter1 && first_,
                   Iter2 && last_ )
      : first( std::move(first_) )
      , last ( std::move(last_ ) )
    {}

    auto begin() const { return first; }
    auto end  () const { return last;   }

    bool empty() const { return first == last; }
    void pop_front() { assert( !empty() ); ++first; }
    decltype(auto) front() const { return *first; }

  private:
    Iter1 first;
    Iter2 last;
  };

} // namespace detail

template <typename Iter1,
          typename Iter2>
auto makeIteratorRange( Iter1 first, Iter2 last )
{
  return detail::IteratorRange<Iter1,Iter2>(
        std::move(first),
        std::move(last) );
}


template <typename Container>
auto makeRange( Container && container )
{
  using std::begin;
  using std::end;
  return makeIteratorRange( begin(container), end(container) );
}


namespace detail
{

  template <typename Container>
  class PushBackRange
  {
  public:
    PushBackRange( Container & container_ )
      : container( container_ )
    {}

    template <typename Value>
    void put( Value && val ) const
    {
      container.push_back( std::forward<Value>(val) );
    }

  private:
    Container & container;
  };

} // namespace detail

template <typename Container>
auto makePushBackRange( Container && container )
{
  return detail::PushBackRange<Container>( container );
}


/********************
 * Range algorithms *
 ********************/

template <typename InputRange,
          typename F>
void for_each( InputRange range, F && f )
{
  for ( ; !range.empty(); range.pop_front() )
    f( range.front() );
}


template <typename InputRange,
          typename OutputRange>
void copy( InputRange in, OutputRange out )
{
  for ( ; !in.empty(); in.pop_front() )
    put( out, in.front() );
}


template <typename RandomAccessRange>
void sort( RandomAccessRange range )
{
  ::std::sort( range.begin(), range.end() );
}


/******************
 * Range adapters *
 ******************/

namespace detail
{
  template <template <typename...> class AdaptedRange,
            typename ...Ts>
  class RangeAdapter
  {
  private:
    std::tuple<Ts...> data;

  public:
    template <typename ...Args>
    RangeAdapter( Ts &&... args )
      : data( std::forward<Ts>(args)... )
    {}

    template <typename OrigRange>
    friend AdaptedRange<OrigRange,Ts...> operator|( OrigRange orig, RangeAdapter adapter )
    {
      const auto f = [&orig]( auto &&...args )
      {
        return AdaptedRange<OrigRange,Ts...>(
              std::move(orig),
              std::move(args)... );
      };
      return cu::apply( f, adapter.data );
    }
  };
} // namespace detail

template <template <typename...> class AdaptedRange,
          typename ...Ts>
auto makeRangeAdapter( Ts...data )
{
  return detail::RangeAdapter<AdaptedRange,Ts...>( std::move(data)... );
}


namespace detail
{

  template <typename InputRange,
            typename F>
  class TransformedRange
  {
  private:
    InputRange in;
    F f;

  public:
    TransformedRange( InputRange && in_, F && f_ )
      : in( std::forward<InputRange>(in_) )
      , f( std::forward<F>(f_) )
    {}

    bool empty() const { return in.empty(); }
    void pop_front() { in.pop_front(); }
    decltype(auto) front() const { return f( in.front() ); }
    decltype(auto) front()       { return f( in.front() ); }
  };

} // namespace detail

template <typename F>
auto transformed( F && f )
{
  return makeRangeAdapter<detail::TransformedRange,F>( std::forward<F>(f) );
}


inline auto moved()
{
  return transformed( []( auto && x ){ return std::move(x); } );
}


} // namespace cu
