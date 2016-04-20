/** @file Defines miscellaneous general purpose functor types.
 * @author Ralph Tandetzky
 */

#pragma once

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

}
