#pragma once

#include <cassert>
#include <memory>
#include <type_traits>

namespace cu
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


template <typename ...Ts>
class VisitableBase
{
public:
  using Visitor = Visitor<Ts...>;
  using ConstVisitor = ConstVisitor<Ts...>;

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

    Result & getResult()
    {
      return result;
    }

  protected:
    F && f;
    Result result{};
  };

  template <typename F, typename Result, typename VisitorBase, typename T, typename ...Ts>
  class VisitorImpl<F, Result, VisitorBase, T, Ts...>
      : public VisitorImpl<F, Result, VisitorBase, Ts...>
  {
    using DirectBase = VisitorImpl<F, Result, VisitorBase, Ts...>;
  public:
    using DirectBase::DirectBase;
    using VisitorBase::visit;
    virtual void visit( T & x ) override
    {
      VisitorImpl<F, Result, VisitorBase>::result =
          VisitorImpl<F, Result, VisitorBase>::f( x );
    }
  };

} // namespace detail


template <typename ...Ts, typename F>
auto visit( VisitableBase<Ts...> & visitable, F && f )
{
  using Result = std::common_type_t<std::result_of_t<F&&(Ts)>...>;
  detail::VisitorImpl<F, Result, Visitor<Ts...>, Ts...> visitor{
    std::forward<F>(f) };
  visitable.accept( visitor );
  return std::move( visitor.getResult() );
}


template <typename ...Ts, typename F>
auto visit( const VisitableBase<Ts...> & visitable, F && f )
{
  using Result = std::common_type_t<std::result_of_t<F&&(Ts)>...>;
  detail::VisitorImpl<F, Result, ConstVisitor<Ts...>, const Ts...> visitor{
    std::forward<F>(f) };
  visitable.accept( visitor );
  return std::move( visitor.getResult() );
}


template <typename VisitableTemplate>
auto clone( const VisitableTemplate & item )
{
  return visit( item, []( const auto & item )
  {
    return std::unique_ptr<VisitableTemplate>(
          std::make_unique<std::decay_t<decltype(item)>>( item ) );
  });
}

} // namespace cu
