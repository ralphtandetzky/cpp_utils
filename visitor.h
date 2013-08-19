/** @file This file contains some C++ template code which makes it easy
    to implement the visitor pattern. 
    
  @author Ralph Tandetzky
  @date 29 Jul 2013 */

#include <cassert>
#include <memory>

namespace cu {

/**********************************
 **  THE SIMPLE VISITOR PATTERN  **
 **********************************/

template <typename ...T>
struct visitor : virtual visitor<T>...
{
	typedef visitor<const T...> const_visitor;

    template <typename F>
    struct impl : visitor, visitor<T>::template impl<F>...
    {
        template <typename...Args>
        explicit impl( Args&&...args ) : f(std::forward<Args>(args)...) {}
        impl( impl & other ) : impl( const_cast<const impl&>(other) ) {}
        impl( impl && ) = default;
        impl( const impl & ) = default;
        impl( const impl && other ) : impl( static_cast<const impl&>(other) ) {}
    private:
        F & get_functor() override { return f; }
        F f;
    };

    template <typename F>
    static impl<F> make_impl( F f )
    {
        return impl<F>( std::move(f) );
    }

    struct visitable_interface
    {
        virtual ~visitable_interface() {}
        virtual void accept( visitor & v ) = 0;
        virtual void accept( const_visitor & v ) const = 0;
    };

    template <typename S>
    struct visitable : visitable_interface
    {
        virtual void accept( visitor & v )
        {
            assert( typeid( *this ) == typeid( S ) );
            static_cast<visitor<S>&>(v).visit(
                static_cast<S&>(*this) );
        }

        virtual void accept( const_visitor & v ) const
        {
            assert( typeid( *this ) == typeid( const S ) );
            static_cast<visitor<const S>&>(v).visit(
                static_cast<const S&>(*this) );
        }
    };
};

template <typename T>
struct visitor<T>
{
	virtual ~visitor() {}
	virtual void visit( T & t ) = 0;

protected:
    template <typename F>
    struct impl : virtual visitor<T>
    {
        void visit( T & t ) override { get_functor()( t ); }
    private:
        virtual F & get_functor() = 0;
    };
};


/***********************************
 **  THE ACYCLIC VISITOR PATTERN  **
 ***********************************/

struct acyclic_visitor_interface
{
    virtual ~acyclic_visitor_interface() {}
};

template <typename ...T>
struct acyclic_visitor : virtual acyclic_visitor<T>...
{
    typedef acyclic_visitor<const T...> const_visitor;

    template <typename F>
    struct impl: acyclic_visitor<T>::template impl<F>...
    {
        template <typename...Args>
        impl( Args&&...args ) : f(std::forward<Args>(args)...) {}
        impl( impl & other ) : impl( const_cast<const impl&>(other) ) {}
        impl( impl && ) = default;
        impl( const impl & ) = default;
        impl( const impl && other ) : impl( static_cast<const impl&>(other) ) {}
    private:
        F & get_functor() override { return f; }
        F f;
    };

    template <typename F>
    static impl<F> make_impl( F f )
    {
        return impl<F>( std::move(f) );
    }
};

template <typename T>
struct acyclic_visitor<T> : virtual acyclic_visitor_interface
{
    virtual void visit( T & ) = 0;

    template <typename F>
    struct impl : virtual acyclic_visitor
    {
        void visit( T & t ) override { get_functor()(t); }
    private:
        virtual F & get_functor() = 0;
    };
};

struct acyclic_visitable_interface
{
    virtual ~acyclic_visitable_interface() {}
    virtual bool try_accept      ( acyclic_visitor_interface & v )       = 0;
    virtual bool try_accept_const( acyclic_visitor_interface & v ) const = 0;
};

template <typename S>
struct acyclic_visitable : virtual acyclic_visitable_interface
{
    virtual bool try_accept( acyclic_visitor_interface & v )
	{
        const auto pv = dynamic_cast<acyclic_visitor<S>*>(&v);
        if ( pv )
        {
            pv->visit( static_cast<S&>(*this) );
            return true;
        }
        return false;
    }

    virtual bool try_accept_const( acyclic_visitor_interface & v ) const
	{
        const auto pv = dynamic_cast<acyclic_visitor<const S>*>(&v);
        if ( pv )
        {
            pv->visit( static_cast<const S&>(*this) );
            return true;
        }
        return false;
	}
};


/***********************
 **  GENERIC CLONING  **
 ***********************/

template <typename Base>
struct cloner
{
    template <typename T>
    void operator()( const T & t )
    { copy = std::unique_ptr<Base>( new T(t) ); }

    std::unique_ptr<Base> copy;
};

template <typename V, typename Base = typename V::visitable_interface>
std::unique_ptr<Base> clone( const typename V::visitable_interface & client )
{
    cloner<Base> c_;
    typename V::const_visitor::template impl<cloner<Base>&> v( c_ );
    client.accept( v );
    return std::move(c_.copy);
}

template <typename V, typename Base>
std::unique_ptr<Base> clone( const acyclic_visitable_interface & client )
{
    cloner<Base> c_;
    typename V::const_visitor::template impl<cloner<Base>&> v( c_ );
    client.try_accept_const( v );
    return std::move(c_.copy);
}


/*************************
 **  GENERIC STREAMING  **
 *************************/

template <typename OutputStream>
struct streamer
{
    streamer( OutputStream & os ) : os(os) {}

    template <typename T>
    void operator()( const T & t ) { os << t; }

private:
    OutputStream & os;
};

template <typename V, typename OutputStream>
OutputStream & print( OutputStream & os, const typename V::visitable_interface & client )
{
    auto v = V::const_visitor::make_impl( streamer<OutputStream>(os) );
    client.accept( v );
    return os;
}

template <typename V, typename OutputStream>
OutputStream & print( OutputStream & os, const acyclic_visitable_interface & client )
{
    auto v = V::const_visitor::make_impl( streamer<OutputStream>(os) );
    client.try_accept_const( v );
    return os;
}

} // namespace cu
