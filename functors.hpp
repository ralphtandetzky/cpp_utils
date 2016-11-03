/** @file Defines miscellaneous general purpose functor types.
 * @author Ralph Tandetzky
 */

#pragma once

#include "functors_fwd.hpp"
#include "swap.hpp"

#include <functional>
#include <memory>
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

  // disable copying and moving
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

  template <typename F>
  class OverloadedFunctor<F> : public F
  {
  public:
    OverloadedFunctor( const F &  f ) : F(           f  ) {}
    OverloadedFunctor(       F && f ) : F( std::move(f) ) {}

    using F::operator();
  };

  template <typename F, typename ...Fs>
  class OverloadedFunctor<F,Fs...>
      : public OverloadedFunctor<F>
      , public OverloadedFunctor<Fs...>
  {
  public:
    template <typename    Arg,
              typename ...Args>
    OverloadedFunctor( Arg  &&    arg,
                       Args &&... args )
      : OverloadedFunctor<F    >( std::forward<Arg >(arg )    )
      , OverloadedFunctor<Fs...>( std::forward<Args>(args)... )
    {}

    using OverloadedFunctor<F    >::operator();
    using OverloadedFunctor<Fs...>::operator();
  };

} // namespace detail

template <typename ...Fs>
auto makeOverloadedFunctor( Fs &&... fs )
{
  return detail::OverloadedFunctor<std::decay_t<Fs>...>( std::forward<Fs>(fs)... );
}


/// This class mimics the behaviour of std::function, except that it
/// does not require the assigned functors to be copyable, but to be
/// movable only. Consequently, @c MoveFunction objects are
/// MoveOnly as well.
template <typename Res,
          typename ...Args>
class MoveFunction<Res(Args...)>
{
public:
  // constructors

  MoveFunction() noexcept
    : payLoad( nullptr, [](void*){} )
  {}

  MoveFunction( std::nullptr_t ) noexcept
    : MoveFunction()
  {}

  MoveFunction(       MoveFunction &  ) = delete;
  MoveFunction( const MoveFunction &  ) = delete;
  MoveFunction( const MoveFunction && ) = delete;

  MoveFunction( MoveFunction && other ) noexcept
    : MoveFunction()
  {
    swap( other );
  }

  template <typename F>
  MoveFunction( F && f )
    : MoveFunction(
        std::forward<F>(f),
        typename std::conditional_t<
          std::is_empty<std::decay_t<F>>::value,
          EmptyPayLoadTag,
          typename std::conditional_t<
            std::is_convertible<F,Res(*)(Args...)>::value,
            FunctionPointerTag,
            NonEmptyPayLoadTag
          >
        >{} )
  {}

  // assignment and swap

  MoveFunction & operator=( MoveFunction other ) noexcept
  {
    other.swap( *this );
    return *this;
  }

  void swap( MoveFunction & other ) noexcept
  {
    std::swap( callPtr, other.callPtr );
    std::swap( payLoad, other.payLoad );
  }

  // operators

  explicit operator bool() const noexcept
  {
    return callPtr;
  }

  Res operator()( Args...args ) const
  {
    if ( *this )
      return callPtr( payLoad.get(), std::forward<Args>(args)... );
    else
      throw std::bad_function_call{};
  }

private:
  struct EmptyPayLoadTag    {};
  struct FunctionPointerTag {};
  struct NonEmptyPayLoadTag {};

  template <typename F>
  MoveFunction( F && f, EmptyPayLoadTag )
    : callPtr
      {
        []( void * payLoad, Args&&...args )
        {
          return (*static_cast<std::remove_reference_t<F>*>(payLoad))(
                std::forward<Args>(args)...);
        }
      }
    , payLoad{ &f, [](void*){} }
  {}

  template <typename F>
  MoveFunction( F && f, FunctionPointerTag )
    : callPtr
      {
        []( void * payLoad, Args&&...args )
        {
          return reinterpret_cast<Res(*)(Args...)>(payLoad)(
                std::forward<Args>(args)...);
        }
      }
    , payLoad{
        reinterpret_cast<void*>(static_cast<Res(*)(Args...)>(f)),
        [](void*){}
      }
  {
    static_assert( sizeof(Res(*)(Args...)) == sizeof(void*), "" );
  }

  template <typename F>
  MoveFunction( F && f, NonEmptyPayLoadTag )
    : callPtr
      {
        []( void * payLoad, Args&&...args )
        {
          return (*static_cast<std::decay_t<F>*>(payLoad))(
                std::forward<Args>(args)... );
        }
      }
    , payLoad{
        new typename std::decay_t<F>( std::forward<F>(f) ),
        []( void * payLoad )
        {
          delete static_cast<typename std::decay_t<F>*>(payLoad);
        }
      }
  {}

  Res (*callPtr)( void *, Args&&... ) = nullptr;
  std::unique_ptr<void,void(*)(void*)> payLoad;
};

} // namespace cu
