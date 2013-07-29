/** @file This file contains some C++ template code which makes it easy
    to implement the visitor pattern. 
    
  @author Ralph Tandetzky
  @date 29 Jul 2013 */

//////////////////////////////////
//  THE SIMPLE VISITOR PATTERN  //
//////////////////////////////////

template <typename ...T>
struct Visitor : virtual Visitor<T>...
{
  typedef Visitor<const T...> ConstVisitor;

	struct VisitableInterface
	{
		virtual ~VisitableInterface {}
		virtual void accept( Visitor & v ) = 0;
		virtual void accept( ConstVisitor & v ) const = 0;
	};

	template <typename S>
	struct Visitable : VisitableInterface
	{
		virtual void accept( Visitor & v )
		{
			assert( typeid( *this ) == typeid( S ) );
			v.visit( static_cast<S&>(*this) );
		}

		virtual void accept( ConstVisitor & v ) const
		{
			assert( typeid( *this ) == typeid( const S ) );
			v.visit( static_cast<const S&>(*this) );			
		}
    };
};

template <typename T>
struct Visitor<T>
{
	virtual ~Visitor() {}
	virtual void visit( T & t ) = 0;
};


///////////////////////////////////
//  THE ACYCLIC VISITOR PATTERN  //
///////////////////////////////////

struct AcyclicVisitor
{
	virtual ~AcyclicVisitor() {}
};

template <typename ...T>
struct AcyclicVisitorImpl : virtual AcyclicVisitorImpl<T>...
{
	typedef AcyclicVisitorImpl<const T...> ConstVisitor;
};

template <typename T>
struct AcyclicVisitorImpl<T> : virtual AcyclicVisitor
{
	virtual void visit( T & ) = 0;
};

struct AcyclicVisitableInterface
{
	virtual ~VisitableInterface {}
	virtual bool tryAccept     ( AcyclicVisitor & v )       = 0;
	virtual bool tryAcceptConst( AcyclicVisitor & v ) const = 0;
};

template <typename S>
struct AcyclicVisitable : virtual AcyclicVisitableInterface
{
	virtual bool tryAccept( AcyclicVisitor & v )
	{
		const auto v_ = dynamic_cast<AcyclicVisitorImpl<S>*>(v);
		if ( !v_ )
			return false;
		v_.visit( static_cast<S&>(*this) );
		return true;
	}

	virtual bool tryAcceptConst( AcyclicVisitor & v ) const
	{
		const auto v_ = dynamic_cast<AcyclicVisitorImpl<const S>*>(v);
		if ( !v_ )
			return false;
		v_->visit( static_cast<const S&>(*this) );
		return true;
	}
};
