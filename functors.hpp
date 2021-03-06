/** @file Defines miscellaneous general purpose functor types.
 * @author Ralph Tandetzky
 *
 * This header defines two generic functor classes which take a function
 * signature as function argument:
 *  - Lambda<Result(Args...)>
 *  - MoveFunction<Result(Args...)> and MoveFunction<Result(Args...)const>
 * These can be passed as function argument types just like
 *  - std::function<Result(Args...)>
 *  - F && where F is a template parameter.
 *  - a C style function pointer Result(*)(Args...).
 *
 * Here's a little overview over the differences of these functor types:
 *
 * Kind of functor     Copyable?   Moveable?  Heap alloc?
 *
 * C-function ptr      yes         yes        no
 * std::function       yes         yes        mostly
 * cu::MoveFunction    no          yes        at most once
 * cu::Lambda          no          no         no
 * template            depends     depends    no
 *
 * Here's a general guideline of when to use which type:
 *  - For maximum performance use the template variant. It allows inlining
 *    and optimizations that follow from that.
 *  - All other variants provide type erasure and hence more runtime
 *    flexibility.
 *  - If you need C compatibility, then C-function pointers are unavoidable.
 *  - If that is not necessary, then the compiler should be able to compile
 *    cu::Lambda with the same performance as the C function pointer.
 *  - Since cu::Lambda is neither movable nor copyable it can only be used
 *    locally. But for this purpose it is perfect: Flexible and super fast.
 *  - cu::MoveFunction and std::function require a heap allocation, if
 *    they are constructed from a stateful functor. This makes them a more
 *    expensive at construction.
 *  - cu::MoveFunction and std::function objects can can be stored in
 *    containers or data fields of classes. This is when they should be used.
 *  - If you require the functor to be copyable, then use std::function.
 *    Otherwise use cu::MoveFunction.
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

  Result operator()( Args ... args ) const
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


template <bool, typename...>
class MoveFunctionBase;

template <bool isCallOpConst,
          typename Res,
          typename ...Args>
class MoveFunctionBase<isCallOpConst, Res(Args...)>
{
  template <typename F>
  using FunctorType = std::conditional_t<isCallOpConst,
    const std::decay_t<F>,std::decay_t<F>>;
  using PayLoadType = FunctorType<void>;

public:
  // constructors

  MoveFunctionBase() noexcept
    : payLoad( nullptr, [](PayLoadType*){} )
  {}

  MoveFunctionBase( std::nullptr_t ) noexcept
    : MoveFunctionBase()
  {}

  MoveFunctionBase(       MoveFunctionBase &  ) = delete;
  MoveFunctionBase( const MoveFunctionBase &  ) = delete;
  MoveFunctionBase( const MoveFunctionBase && ) = delete;

  MoveFunctionBase( MoveFunctionBase && other ) noexcept
    : MoveFunctionBase()
  {
    swap( other );
  }

  template <typename F>
  MoveFunctionBase( F && f )
    : MoveFunctionBase(
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

  MoveFunctionBase & operator=( MoveFunctionBase other ) noexcept
  {
    other.swap( *this );
    return *this;
  }

  void swap( MoveFunctionBase & other ) noexcept
  {
    std::swap( callPtr, other.callPtr );
    std::swap( payLoad, other.payLoad );
  }

  // operators

  explicit operator bool() const noexcept
  {
    return callPtr;
  }

  Res (*callPtr)( PayLoadType *, Args&&... ) = nullptr;
  std::unique_ptr<PayLoadType,void(*)(PayLoadType*)> payLoad;

private:
  struct EmptyPayLoadTag    {};
  struct FunctionPointerTag {};
  struct NonEmptyPayLoadTag {};

  template <typename F>
  MoveFunctionBase( F && f, EmptyPayLoadTag )
    : callPtr
      {
        []( PayLoadType * payLoad, Args&&...args )
        {
          return (*static_cast<FunctorType<F>*>(payLoad))(
                std::forward<Args>(args)...);
        }
      }
    , payLoad{ &f, [](PayLoadType*){} }
  {}

  template <typename F>
  MoveFunctionBase( F && f, FunctionPointerTag )
    : callPtr
      {
        []( PayLoadType * payLoad, Args&&...args )
        {
          return reinterpret_cast<Res(*)(Args...)>(payLoad)(
                std::forward<Args>(args)...);
        }
      }
    , payLoad{
        reinterpret_cast<PayLoadType*>(static_cast<Res(*)(Args...)>(f)),
        [](PayLoadType*){}
      }
  {
    static_assert( sizeof(Res(*)(Args...)) == sizeof(PayLoadType*), "" );
  }

  template <typename F>
  MoveFunctionBase( F && f, NonEmptyPayLoadTag )
    : callPtr
      {
        []( PayLoadType * payLoad, Args&&...args )
        {
          return (*static_cast<FunctorType<F>*>(payLoad))(
                std::forward<Args>(args)... );
        }
      }
    , payLoad{
        new typename std::decay_t<F>( std::forward<F>(f) ),
        []( PayLoadType * payLoad )
        {
          delete static_cast<FunctorType<F>*>(payLoad);
        }
      }
  {}
};


/// This class mimics the behaviour of std::function, except that it
/// does not require the assigned functors to be copyable, but to be
/// movable only. Consequently, @c MoveFunction objects are
/// MoveOnly as well.
template <typename Res,
          typename ...Args>
class MoveFunction<Res(Args...)>
    : private MoveFunctionBase<false, Res(Args...)>
{
  using Base = MoveFunctionBase<false, Res(Args...)>;
public:
  using Base::Base;

  MoveFunction() = default;
  MoveFunction( MoveFunction && ) = default;

  MoveFunction & operator=( MoveFunction other )
  {
    swap( other );
    return *this;
  }

  void swap( MoveFunction & other )
  {
    Base::swap( other );
  }

  using Base::operator bool;

  Res operator()( Args...args )
  {
    if ( *this )
      return this->callPtr( this->payLoad.get(), std::forward<Args>(args)... );
    else
      throw std::bad_function_call{};
  }
};


template <typename Res,
          typename ...Args>
class MoveFunction<Res(Args...) const>
    : private MoveFunctionBase<true, Res(Args...)>
{
  using Base = MoveFunctionBase<true, Res(Args...)>;
public:
  using Base::Base;

  MoveFunction() = default;
  MoveFunction( MoveFunction && ) = default;

  MoveFunction & operator=( MoveFunction other )
  {
    swap( other );
    return *this;
  }

  void swap( MoveFunction & other )
  {
    Base::swap( other );
  }

  using Base::operator bool;

  Res operator()( Args...args ) const
  {
    if ( *this )
      return this->callPtr( this->payLoad.get(), std::forward<Args>(args)... );
    else
      throw std::bad_function_call{};
  }
};

} // namespace cu
