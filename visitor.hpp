#pragma once

#include "cow_ptr.hpp"
#include "functors.hpp"
#include "meta_programming.hpp"

#include <cassert>
#include <experimental/optional>
#include <memory>
#include <type_traits>
#include <vector>

namespace cu
{

namespace detail
{

  template <typename ...Ts>
  class Visitor;

  template <>
  class Visitor<>
  {
  protected:
    void visit() = delete;
  };

  template <typename T, typename ...Ts>
  class Visitor<T,Ts...> : public Visitor<Ts...>
  {
  public:
    virtual ~Visitor() = default;

    using Visitor<Ts...>::visit;
    virtual void visit( T& ) = 0;
  };

  template <typename ...Ts>
  using ConstVisitor = Visitor<const Ts...>;

} // namespace detail

template <typename ...Ts>
class VisitableBase
{
public:
  using Visitor = detail::Visitor<Ts...>;
  using ConstVisitor = detail::ConstVisitor<Ts...>;

  virtual ~VisitableBase() = default;

  virtual void accept( Visitor & visitor ) = 0;
  virtual void accept( ConstVisitor & visitor ) const = 0;
};


/// Base should derive from @c VisitableBase<...,Derived,...>.
template <typename Derived, typename Base>
class VisitableImpl
    : public Base
{
public:
  using Visitor = typename Base::Visitor;
  using ConstVisitor = typename Base::ConstVisitor;

  using Base::Base;

  virtual void accept( Visitor & visitor ) override
  {
    assert( dynamic_cast<Derived*>(this) &&
            "The dynamic type of this object must be 'Derived' "
            "or inherit from it." );
    visitor.visit( static_cast<Derived&>(*this) );
  }

  virtual void accept( ConstVisitor & visitor ) const override
  {
    assert( dynamic_cast<const Derived*>(this) &&
            "The dynamic type of this object must be 'Derived' "
            "or inherit from it." );
    visitor.visit( static_cast<const Derived&>(*this) );
  }
};


namespace detail
{

  template <typename T, bool isDefaultConstructible>
  struct OptionalIfNotDefaultConstructibleImpl;

  template <typename T>
  struct OptionalIfNotDefaultConstructibleImpl<T,true>
  {
    using type = T;
    template <typename U>
    static decltype(auto) moveOutValue( U && x ) { return std::move(x); }
  };

  template <typename T>
  struct OptionalIfNotDefaultConstructibleImpl<T,false>
  {
    using type = std::experimental::optional<T>;
    template <typename U>
    static decltype(auto) moveOutValue( U && x ) { return std::move(x.value()); }
  };

  template <typename T>
  struct OptionalIfNotDefaultConstructible
      :  OptionalIfNotDefaultConstructibleImpl<
            T,
            std::is_default_constructible<T>::value>
  {};

  template <typename ...Ts>
  class VisitorImpl;

  template <typename F, typename Result, typename VisitorBase>
  class VisitorImpl<F, Result, VisitorBase>
      : public VisitorBase
  {
  public:
    VisitorImpl( F && f_ )
      : f(std::forward<F>(f_))
    {}

    Result && getResult()
    {
      return OptionalIfNotDefaultConstructible<Result>::moveOutValue( result );
    }

  protected:
    template <typename T>
    void doVisit( T && arg )
    {
      result = std::forward<F>(f)( std::forward<T>(arg) );
    }

  private:
    typename OptionalIfNotDefaultConstructible<Result>::type result{};
    F && f;
  };

  template <typename F,
            typename VisitorBase>
  class VisitorImpl<F, void, VisitorBase>
      : public VisitorBase
  {
  public:
    VisitorImpl( F && f_ )
      : f(std::forward<F>(f_))
    {}

    void getResult()
    {}

  protected:
    template <typename T>
    void doVisit( T && arg )
    {
      std::forward<F>(f)( std::forward<T>(arg) );
    }

  private:
    F && f;
  };

  template <typename F,
            typename Result,
            typename VisitorBase,
            typename T,
            typename ...Ts>
  class VisitorImpl<F, Result, VisitorBase, T, Ts...>
      : public VisitorImpl<F, Result, VisitorBase, Ts...>
  {
    using DirectBase = VisitorImpl<F, Result, VisitorBase, Ts...>;
  public:
    using DirectBase::DirectBase;
    using VisitorBase::visit;
    virtual void visit( T & x ) override
    {
      this->doVisit( x );
    }
  };

} // namespace detail

template <typename ...Ts, typename F>
auto visit( VisitableBase<Ts...> & visitable, F && f )
{
  using Result = std::common_type_t<std::result_of_t<F&&(Ts&)>...>;
  detail::VisitorImpl<F, Result, detail::Visitor<Ts...>, Ts...> visitor{
    std::forward<F>(f) };
  visitable.accept( visitor );
  return visitor.getResult();
}

template <typename ...Ts, typename F>
auto visit( const VisitableBase<Ts...> & visitable, F && f )
{
  using Result = std::common_type_t<std::result_of_t<F&&(Ts&)>...>;
  detail::VisitorImpl<F, Result, detail::ConstVisitor<Ts...>, const Ts...> visitor{
    std::forward<F>(f) };
  visitable.accept( visitor );
  return visitor.getResult();
}

template <typename ...Ts, typename ...Fs>
decltype(auto) visit( VisitableBase<Ts...> & visitable, Fs &&... fs )
{
  return visit( visitable, makeOverloadedFunctor( fs... ) );
}

template <typename ...Ts, typename ...Fs>
decltype(auto) visit( const VisitableBase<Ts...> & visitable, Fs &&... fs )
{
  return visit( visitable, makeOverloadedFunctor( fs... ) );
}


namespace detail
{

  template <typename VisitableTemplate, typename ...Ts>
  decltype(auto) makeGenericCloner( const VisitableBase<Ts...> & )
  {
    return []( const auto & item )
    {
      return cu::make_cow<VisitableTemplate,
                          std::decay_t<decltype(item)>>( item );
    };
  }

  template <typename VisitableTemplate, typename Derived, typename Base>
  decltype(auto) makeGenericCloner( const VisitableImpl<Derived,Base> & )
  {
    return []( const auto & item )
    {
      return cu::make_cow<Base,std::decay_t<decltype(item)>>( item );
    };
  }

} // namespace detail

template <typename VisitableTemplate>
auto clone( const VisitableTemplate & item )
  -> decltype(
         visit( item, detail::makeGenericCloner<VisitableTemplate>( item ) ) )
{
  return visit( item, detail::makeGenericCloner<VisitableTemplate>( item ) );
}

template <typename VisitableTemplate>
auto clone( const std::vector<std::unique_ptr<VisitableTemplate> > & v )
  -> decltype( (void)clone( *v[0] ),
               std::unique_ptr<std::vector<std::unique_ptr<VisitableTemplate> > >() )
{
  auto result = std::make_unique<std::vector<std::unique_ptr<VisitableTemplate> > >();
  for ( const auto & x : v )
    result->push_back( clone( *x ) );

  return result;
}


namespace detail
{

  struct IsEqualFunctor
  {
    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const
    {
      return lhs == rhs;
    }

    template <typename T1, typename T2>
    bool operator()( const T1 &, const T2 & ) const
    {
      return false;
    }
  };

} // namespace detail

template <typename VisitableTemplate>
bool isEqual( const VisitableTemplate & lhs, const VisitableTemplate & rhs )
{
  return visit( lhs, [&rhs]( const auto & lhs )
  {
    return visit( rhs, [&lhs]( const auto & rhs )
    {
      return detail::IsEqualFunctor()( lhs, rhs );
    });
  });
}


template <typename T>
struct VisitableTag { using type = T; };

namespace detail
{
  template <typename ...ArgsOfBase>
  auto makeVisitableTagTupleImpl(
      const VisitableBase<ArgsOfBase...> & )
  {
    return std::make_tuple( VisitableTag<ArgsOfBase>{}... );
  }
} // namespace detail

/// Returns a @c std::tuple<VisitableTag<Ts>...> for template meta programming.
///
/// The template parameter pack @c Ts is chosen such that
/// @c DerivedFromVisitableBase derives from @c VisitableBase<Ts...>.
///
/// The returned value can be used for template meta programming, in particular
/// with @c cu::for_each().
///
/// @example If all derived classes of visitable class @c MyVisitable are
/// default constructible, then a @c std::vector<std::unique_ptr<MyVisitable>
/// can be created easily by writing the following:
///   @code
///     std::vector<std::unique_ptr<MyVisitable> > generateAllMyVisitables()
///     {
///       std::vector<std::unique_ptr<MyVisitable>> result;
///       cu::for_each(
///             cu::makeVisitableTagTuple<MyVisitable>(),
///             [&]( auto tag )
///       {
///         result.push_back( std::make_unique<typename decltype(tag)::type>() );
///       });
///       return result;
///     }
///   @endcode
/// Note that the template function @c generateAllDerivedVisitables() does
/// exactly that.
template <typename DerivedFromVisitableBase>
auto makeVisitableTagTuple()
{
  return decltype( detail::makeVisitableTagTupleImpl(
                     std::declval<DerivedFromVisitableBase>() ) ){};
}


/// Creates a vector of derived visitable types of @c VisitableBaseType.
///
/// If @c VisitableBaseType derived from @c VisitableBase<Ts...>, then
/// a vector is generated, which contains the following elements:
///   @code
///     { std::make_unique<Ts>()... }
///   @endcode
/// Note
template <typename VisitableBaseType>
std::vector<std::unique_ptr<VisitableBaseType> > generateAllDerivedVisitables()
{
  std::vector<std::unique_ptr<VisitableBaseType>> result;
  cu::for_each(
        cu::makeVisitableTagTuple<VisitableBaseType>(),
        [&]( auto tag )
  {
    result.push_back( std::make_unique<typename decltype(tag)::type>() );
  });
  return result;
}

} // namespace cu
