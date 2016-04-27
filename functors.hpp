/** @file Defines miscellaneous general purpose functor types.
 * @author Ralph Tandetzky
 */

#pragma once

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

} // namespace cu
