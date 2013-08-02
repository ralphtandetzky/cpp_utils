/** @file This file contains some C++ template code which makes it easy
    to implement the visitor pattern. 
    
  @author Ralph Tandetzky
  @date 29 Jul 2013 */

#include <typeinfo>
#include <cassert>

/**********************************
 **  THE SIMPLE VISITOR PATTERN  **
 **********************************/

template <typename ...T>
struct Visitor : virtual Visitor<T>...
{
	typedef Visitor<const T...> ConstVisitor;

    template <template<typename> class Policy>
    struct Impl : Visitor, Visitor<T>::template Impl<Policy>... {};

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

    template <template<typename> class Policy>
    struct Impl : virtual Visitor<T>
    {
        void visit( T & t ) override
        {
            Policy<T>::f( t );
        }
    };
};


/***********************************
 **  THE ACYCLIC VISITOR PATTERN  **
 ***********************************/

struct AcyclicVisitorInterface
{
    virtual ~AcyclicVisitorInterface() {}
};

template <typename ...T>
struct AcyclicVisitor : virtual AcyclicVisitor<T>...
{
    typedef AcyclicVisitor<const T...> ConstVisitor;

    template <template<typename> class Policy>
    struct Impl : AcyclicVisitor<T>::template Impl<Policy>... {};
};

template <typename T>
struct AcyclicVisitor<T> : virtual AcyclicVisitorInterface
{
    typedef AcyclicVisitor<const T> ConstVisitor;
    virtual void visit( T & ) = 0;

    template <template<typename> class Policy>
    struct Impl : AcyclicVisitor
    {
        void visit( T & t ) override
        {
            Policy<T>::f(t);
        }
    };
};

struct AcyclicVisitableInterface
{
    virtual ~AcyclicVisitableInterface() {}
    virtual bool tryAccept     ( AcyclicVisitorInterface & v )       = 0;
    virtual bool tryAcceptConst( AcyclicVisitorInterface & v ) const = 0;
};

template <typename S>
struct AcyclicVisitable : virtual AcyclicVisitableInterface
{
    virtual bool tryAccept( AcyclicVisitorInterface & v )
	{
        const auto pv = dynamic_cast<AcyclicVisitor<S>*>(&v);
        if ( !pv )
			return false;
        pv->visit( static_cast<S&>(*this) );
		return true;
	}

    virtual bool tryAcceptConst( AcyclicVisitorInterface & v ) const
	{
        const auto pv = dynamic_cast<AcyclicVisitor<const S>*>(&v);
        if ( !pv )
			return false;
        pv->visit( static_cast<const S&>(*this) );
		return true;
	}
};
