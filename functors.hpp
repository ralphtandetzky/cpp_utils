/** @file Defines miscellaneous general purpose functor types.
 * @author Ralph Tandetzky
 */

#pragma once

#include "swap.hpp"

#include <utility>

namespace cu
{

/// A functor class whose function call operator does nothing.
///
/// This is a class that is needed to work around the gcc bug, that the
/// lambda @c [](auto&&...){} required at least one argument.
struct NoOpFunctor
{
  template <typename ...Ts>
  void operator()( Ts&&... ) const
  {
  }
};


struct ForwardingFunctor
{
  template <typename T>
  decltype(auto) operator()( T && x ) const
  {
    return std::forward<T>(x);
  }
};


template <typename Signature>
class Lambda;

template <typename Result, typename ... Args>
class Lambda<Result(Args...)>
{
public:
  template <typename F>
  Lambda( const F & functor )
    : workLoad( &functor )
    , f( []( const void * workLoad, Args &&... args ) -> Result
         {
            return (*(static_cast<const F*>(workLoad)))( std::forward<Args>(args)... );
         } )
  {
  }

  // disable copying
  Lambda( const Lambda & ) = delete;
  Lambda & operator=( Lambda ) = delete;

  Result operator()( Args &&... args ) const
  {
    return f( workLoad, std::forward<Args>(args)... );
  }

private:
  const void * workLoad = nullptr;
  Result (*f)(const void*,Args&&...) = nullptr;
};


namespace detail
{

  template <typename ...Fs>
  class OverloadedFunctor;

  template <>
  class OverloadedFunctor<>
  {
  public:
    template <typename ...Ts,
              typename = std::enable_if_t<false,std::tuple<Ts...>>> // always disabled.
    void operator()( Ts &&... );
  };

  template <typename F, typename ...Fs>
  class OverloadedFunctor<F,Fs...>
      : public OverloadedFunctor<Fs...>
  {
  private:
    F f;
    using Base = OverloadedFunctor<Fs...>;

  public:
    OverloadedFunctor( F && f_, Fs &&... fs )
      : Base( std::forward<Fs>(fs)... )
      , f( std::forward<F>(f_) )
    {}

    using Base::operator();

    template <typename ...Ts>
    auto operator()( Ts &&... args )
      -> decltype( f( std::forward<Ts>(args)... ) )
    {
      return f( std::forward<Ts>(args)... );
    }

    template <typename ...Ts>
    auto operator()( Ts &&... args ) const
      -> decltype( f( std::forward<Ts>(args)... ) )
    {
      return f( std::forward<Ts>(args)... );
    }
  };

} // namespace detail

template <typename ...Fs>
auto makeOverloadedFunctor( Fs &&... fs )
{
  return detail::OverloadedFunctor<Fs...>( std::forward<Fs>(fs)... );
}


/// This class mimics the behaviour of std::function, except that it
/// does not require the assigned functors to be copyable, but to be
/// movable only. Consequently, @c function_move_only objects are
/// MoveOnly as well.
template <typename T>
class function_move_only;

template <typename Res,
          typename ...Args>
class function_move_only<Res(Args...)>
{
public:
  function_move_only() noexcept
  {}

  function_move_only( std::nullptr_t ) noexcept
  {}

  function_move_only( function_move_only && other ) noexcept
  {
    swap( other );
  }

  template <typename F>
  function_move_only( F && f )
    : callPtr( makeCallPtr<F>() )
    , cleanUpPtr( makeCleanUpPtr<F>() )
    , payLoad( makePayLoad( CU_FWD(f) ) )
  {}

  ~function_move_only()
  {
    cleanUp();
  }

  function_move_only & operator=( std::nullptr_t ) noexcept
  {
    cleanUp();
  }

  function_move_only & operator=( function_move_only && other ) noexcept
  {
    cleanUp();
    swap( other );
  }

  template <typename F>
  function_move_only & operator=( F && f )
  {
    function_move_only( CU_FWD(f) ).swap( *this );
  }

  void swap( function_move_only & other ) noexcept
  {
    cu::swap( callPtr   , other.callPtr    );
    cu::swap( cleanUpPtr, other.cleanUpPtr );
    cu::swap( payLoad   , other.payLoad    );
  }

  explicit operator bool() const noexcept;
  Res operator()( Args...args ) const
  {
    callPtr( payLoad, CU_FWD(args)... );
  }

private:
  template <typename F>
  static Res (*makeCallPtr())( void *, Args&&... )
  {
    return []( void * payLoad, Args&&...args )
    {
      (*static_cast<typename std::decay<F>::type*>(payLoad))( CU_FWD(args)... );
    };
  }

  template <typename F>
  static void (*makeCleanUpPtr())( void* )
  {
    return []( void * payLoad )
    {
      delete static_cast<typename std::decay<F>::type*>(payLoad);
    };
  }

  template <typename F>
  static void * makePayLoad( F && f )
  {
    return new typename std::decay<F>::type( CU_FWD(f) );
  }

  void cleanUp() noexcept
  {
    if ( cleanUpPtr )
      cleanUpPtr( payLoad );
    callPtr    = nullptr;
    cleanUpPtr = nullptr;
    payLoad    = nullptr;
  }

  Res (*callPtr)( void *, Args&&... ) = nullptr;
  void (*cleanUpPtr)( void * ) = nullptr;
  void * payLoad = nullptr;
};

} // namespace cu
