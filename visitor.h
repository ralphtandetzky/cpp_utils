/** @file This file contains some C++ template code which makes it easy
    to implement the visitor pattern. 
    
  @author Ralph Tandetzky
  @date 29 Jul 2013 */

#include <typeinfo>
#include <cassert>

#include <utility>

/**********************************
 **  THE SIMPLE VISITOR PATTERN  **
 **********************************/

template <typename ...T>
struct Visitor : virtual Visitor<T>...
{
	typedef Visitor<const T...> ConstVisitor;

    template <typename F>
    struct Impl : Visitor, Visitor<T>::template Impl<F>...
    {
        template <typename...Args>
        explicit Impl( Args&&...args ) : f(std::forward<Args>(args)...) {}
        Impl( Impl & other ) : Impl( const_cast<const Impl&>(other) ) {}
        Impl( Impl && ) = default;
        Impl( const Impl & ) = default;
        Impl( const Impl && other ) : Impl( static_cast<const Impl&>(other) ) {}
    private:
        F & getFunctor() override { return f; }
        F f;
    };

    template <typename F>
    static Impl<F> makeImpl( F f )
    {
        return Impl<F>( std::move(f) );
    }

    struct VisitableInterface
    {
        virtual ~VisitableInterface() {}
        virtual void accept( Visitor & v ) = 0;
        virtual void accept( ConstVisitor & v ) const = 0;
    };

	template <typename S>
    struct Visitable : VisitableInterface
    {
        virtual void accept( Visitor & v )
        {
            assert( typeid( *this ) == typeid( S ) );
            static_cast<Visitor<S>&>(v).visit(
                static_cast<S&>(*this) );
        }

        virtual void accept( ConstVisitor & v ) const
        {
            assert( typeid( *this ) == typeid( const S ) );
            static_cast<Visitor<const S>&>(v).visit(
                static_cast<const S&>(*this) );
        }
    };
};

template <typename T>
struct Visitor<T>
{
	virtual ~Visitor() {}
	virtual void visit( T & t ) = 0;

protected:
    template <typename F>
    struct Impl : virtual Visitor<T>
    {
        void visit( T & t ) override { getFunctor()( t ); }
    private:
        virtual F & getFunctor() = 0;
    };
};


/***********************************
 **  THE ACYCLIC VISITOR PATTERN  **
 ***********************************/

template <typename Tag>
struct AcyclicVisitorInterface
{
    virtual ~AcyclicVisitorInterface() {}
};

template <typename Tag, typename ...T>
struct AcyclicVisitor : virtual AcyclicVisitor<Tag, T>...
{
    typedef AcyclicVisitor<Tag,const T...> ConstVisitor;

    template <typename F>
    struct Impl: AcyclicVisitor<Tag,T>::template Impl<F>...
    {
        template <typename...Args>
        Impl( Args&&...args ) : f(std::forward<Args>(args)...) {}
        Impl( Impl & other ) : Impl( const_cast<const Impl&>(other) ) {}
        Impl( Impl && ) = default;
        Impl( const Impl & ) = default;
        Impl( const Impl && other ) : Impl( static_cast<const Impl&>(other) ) {}
    private:
        F & getFunctor() override { return f; }
        F f;
    };

    template <typename F>
    static Impl<F> makeImpl( F f )
    {
        return Impl<F>( std::move(f) );
    }
};

template <typename Tag, typename T>
struct AcyclicVisitor<Tag,T> : virtual AcyclicVisitorInterface<Tag>
{
    virtual void visit( T & ) = 0;

    template <typename F>
    struct Impl : AcyclicVisitor
    {
        void visit( T & t ) override { getFunctor()(t); }
    private:
        virtual F & getFunctor() = 0;
    };
};

template <typename Tag>
struct AcyclicVisitableInterface
{
    virtual ~AcyclicVisitableInterface() {}
    virtual bool tryAccept     ( AcyclicVisitorInterface<Tag> & v )       = 0;
    virtual bool tryAcceptConst( AcyclicVisitorInterface<Tag> & v ) const = 0;
};

template <typename Tag, typename S>
struct AcyclicVisitable : virtual AcyclicVisitableInterface<Tag>
{
    virtual bool tryAccept( AcyclicVisitorInterface<Tag> & v )
	{
        const auto pv = dynamic_cast<AcyclicVisitor<Tag,S>*>(&v);
        if ( !pv )
			return false;
        pv->visit( static_cast<S&>(*this) );
		return true;
	}

    virtual bool tryAcceptConst( AcyclicVisitorInterface<Tag> & v ) const
	{
        const auto pv = dynamic_cast<AcyclicVisitor<Tag,const S>*>(&v);
        if ( !pv )
			return false;
        pv->visit( static_cast<const S&>(*this) );
		return true;
	}
};

