/** @file cow_ptr.h

    This file works with the following compilers
      - gcc 4.6.3 with the command line option -std=c++0x.

  @author Ralph Tandetzky
  @date 04 Feb 2013 */

//             Copyright Ralph Tandetzky 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef COW_PTR_H
#define COW_PTR_H

#include <atomic>
#include <cassert>

namespace cow
{

/// Functor which deletes pointers.
struct default_deleter
{
    template <typename T>
    void operator()( T * p ) const noexcept
    {
        delete p;
    }
};


/// Functor which creates a new copy of an object pointed to.
struct default_cloner
{
    template <typename T>
    T * operator()( const T * p ) const
    {
        return new T(*p);
    }
};


/// Smart pointer class for implementing copy-on-write.
/** This is a reference-counted smart pointer class with strong value
    semantics. If you copy a cow pointer (copy-on-write pointer), then it
    will be as if the pointed-to object has been copied. Internally copying
    the object will only be performed when non-const operations are applied
    and the reference count is at least 2. Here's an example to illustrate
    that.
      @code
        cow_ptr<X> a( new X );
        cow_ptr<X> b( a ); // b points to the same object as a internally.
        cow_ptr<X> c;
        c = b     // a, b and c all point to the same object internally.
        b->mutate(); // A new copy of b is created and then mutate is called.
                     // a and c still point to the same object internally.
        b->mutateAgain(); // No extra copying is performed here.
      @endcode

    There are some remarkable use-cases for cow pointers.
      * They are well suited as pimpl pointers for classes with value
        semantics. With most other smart pointers it is always necessary to
        reimplement the copy constructor, and the copy assignment operator,
        sometimes also the destructor. This is not necessary here and you'll
        get the right semantics and even the additional performance gain due
        to lazy copying. A futher advantage is that constness of cow pointer
        member functions propagates to the @c Impl object naturally making it
        easier to write const-correct code.
          @code
            // central_class_with_value_semantics.h
            class CentralClassWithValueSemantics
            {
            public:
                // ... public interface goes here ... //
            private:
                struct Impl; // forward declaration
                cow_ptr<Impl> m;
            };

            // central_class_with_value_semantics.cpp
            struct CentralClassWithValueSemantics::Impl
            {
                // ... definition of hidden members goes here ... //
            }
          @endcode
        This is called the pimpl idiom, or private implementation idiom,
        handle body idiom, or compiler firewall idiom.
      * For classes with members whose copy-operations are expensive and/or
        which take a lot of space in memory, these members can be wrapped
        in a cow pointer. An example are matrix or image classes whose data
        might be stored in a @c std::vector. The matrix header information
        may be changed without deep copy. This boosts performance and optimizes
        memory usage, but at the same time retains the value semantics one
        feels comfortable with.
          @code
            class Matrix
            {
            public:
                // ... public interface goes here ... //
            private:
                size_t nRows, nCols; // can be touched without deep copy.
                cow_ptr<std::vector<float>> data; // copy-on-write
            }
          @endcode
        In such classes the default version of copy and move construction
        will usually work just fine.
      * You can add cloning to a class hierarchy from the outside. With
          @code
            cow_ptr<Base> a( new Derived1 );
            cow_ptr<Base> b( new Derived2 );
            cow_ptr<Base> c;
            c = a; // performs a shallow copy.
            c->doSomething(); // makes a deep copy of a as a Derived1 class.
                // There is no slicing involved.
          @endcode
        you copy @c Base objects polymorphically. The class @c Base can even
        be abstract here. It is only required that @c Derived1 and
        @c Derived2 be @c CopyConstructible.
      * You can create arrays with elements that retain polymorphic behaviour
        and have genuine value sematics at the same time with
          @code
            std::vector<cow_ptr<Base>> polymorphic_array_with_value_semantics;
          @endcode

    There is only a const version of the member function @c get() which
    propagates constness, because a non-const version of the member function
    @c get() would have two major pitfalls:
      * The function
          @code
            T * cow_ptr<T>::get();
          @endcode
        would have to make an internal copy, if the reference count is greater
        than 1, since the calling code is able to modify the object pointed to.
        It would happen too quickly by accident to call the non-const version
        instead of the const version.
      * The function would let a pointer escape which can be used to modify
        the object pointed to after the cow_ptr<T> has been copied without any
        chance for the cow pointer to notice this change. For instance
          @code
            cow_ptr<T> p( new T );
            T * raw = p.get();
            cow_ptr<T> q( p );
            raw->modify(); // modifies the guts of both p and q
          @endcode
        would break the copy-on-write semantics of the cow pointer.
    In order to make the user interface of the class easy to use correctly and
    hard to use incorrectly, a member template function @c apply() is given.
    This function takes a functor as argument which takes a @c T* argument.
    It makes an internal copy of the object pointed to, if the reference count
    is at least 2 and then it calls the functor with the stored @c T* pointer.
    Hence the code above would be written like this:
      @code
        cow_ptr<T> p( new T );
        cow_ptr<T> q( p );
        p.apply( [](T*raw){ raw->modify(); } ); // performes deep copy first.
      @endcode
    It is still possible to let pointers escape and thereby break the
    copy-on-write semantics as mentioned above, but it is harder. Using the
    macro @c COW_APPLY the last line can be simplified to
      @code
        COW_APPLY(p) { p->modify(); };
      @endcode
    Unfortunately the semicolon at the end of the line is necessary. For
    constant operations there's an equivalent macro @c COW_APPLY_CONST.

    Similar to @c std::make_shared and the soon coming @c std::make_unique
    there is an equivalent @c make_cow which makes code faster, more
    exception-safe and less repetitive. You can write
      @code
        auto p = make_cow<Base,Derived>( ... );
      @endcode
    instead of
      @code
        cow_ptr<Base> p( new Derived(...) );
      @endcode
    which has the advantage that only one allocation with @c new is performed
    instead of two. If @c Base and @c Derived are the same, then you can omit
    the second template argument to @c make_cow, which spares you of repeating
    the type. (To see why this is good practice in order to obtain
    exception-safe code see http://herbsutter.com/gotw/_102/)

    Requirements on @c T: The only requirement is that @c T must be
    @c CopyConstructible and @c operator new() must be accessible, if a
    @c default_cloner is used.

    Exception-safety: Construction with a given pointer and various non-const
    operations have the strong exception guarantee and may throw. All other
    operations (including copy construction and copy assignment) do not throw.
    This may change the exception guarantees of a class, if cow pointers are
    used as pimpl pointers.
    @c cow_ptr<T> is exception-neutral, which means that all raised exceptions
    propagate out from cow_ptr<T> without being handled or swallowed.
    Furthermore, @c cow_ptr<T> does not throw itself, but only called code may
    throw.

    Thread-safety: Here we speak of 'reads' as const operations and 'writes'
    as non-const operations. Each cow pointer object can be safely read by
    multiple threads at the same time as long as no thread is writing to the
    object. It is safe to use cow pointers in one thread as long as all copies
    of that cow pointer and any references to the pointed to object stay within
    that thread.
    If the clone operation (by default composed of @c T::operator new() and
    the copy constructor of @c T) of a cow pointer object is thread-safe,
    then the cow pointer can be arbitrarily written to by one thread, even
    if another cow pointer pointing to the same object is accessed by a
    different thread at the same time. If one thread modifies a cow pointer,
    then it must be protected from any read or write access by other threads
    for that time.
    These above guarantees do not include guarantees about the pointed to
    object. They only concern the cow pointer object. The following statement
    gives guarantees on the complete thread-safety of cow pointers including
    access to the pointed-to object: If
      - @c T objects can be read simultaneously by different threads as long
        as no one is writing to it and
      - every thread can write to @c T objects as long as all writing
        operations are serialized for each individual object
    then the same two guarantees hold for the complete use of cow_ptr<T>,
    as long as no raw pointer or reference ever escapes the cow pointer in a
    way that the object could be modified from the outside. This includes all
    read and write access to the pointed-to object. */
template <typename T>
class cow_ptr
{
public:
    //////////////////
    // constructors //
    //////////////////

    /// Default constuctor.
    cow_ptr() noexcept;

    /// Copy constructor. Never throws!
    cow_ptr( const cow_ptr & other ) noexcept;

    /// Move constructor.
    cow_ptr( cow_ptr && other ) noexcept;

    /// Obtains ownership of @c p.
    /** When the reference count drops to zero, then @c deleter(p) will
        be called. Hence @c deleter(p) must be a valid expression and
        it must not throw. Moreover, @c cloner(const_cast<const Y *>(p)) must
        return a pointer that is implicitely convertible to @c T* and may serve
        as an argument to a copy of @c cloner and so forth. It is the
        responsibility of @c cloner to create a new copy of @c *p, whenever
        copying is necessary. Both @c deleter and @c cloner must be
        @c MoveConstructible and @c CopyConstructible. */
    template <typename Y
            , typename D = default_deleter
            , typename C = default_cloner>
    explicit cow_ptr( Y * p
          , D deleter = default_deleter()
          , C cloner = default_cloner() );

    ////////////////
    // destructor //
    ////////////////

    /// Destructor.
    ~cow_ptr() noexcept;

    /////////////////////////
    // swap and assignment //
    /////////////////////////

    /// Non-throwing swap.
    void swap( cow_ptr & other ) noexcept;

    /// Copy assignment. Never throws!
    cow_ptr & operator=( cow_ptr other ) noexcept;

    /////////////////////////////////
    // access to object pointed to //
    /////////////////////////////////

    /// Returns the contained pointer propagating constness.
    const T * get() const noexcept;

    /// Calls @c f(p) where @c p is the contained pointer after making an
    /// internal copy if necessary.
    /** An internal copy is made, if the reference count is 2 or greater.
        This function can be used to make sure the reference count is 1 by
        passing a functor that does nothing. However, this should normally
        not be necessary. */
    template <typename Func>
    void apply( Func f );

    /// Same as @c apply_const().
    /** Prefer to use @c apply_const() wherever possible to avoid calling the
        non-const overload by accident and to make your intent clearer. */
    template <typename Func>
    void apply( Func f ) const; // noexcept( noexcept( f(get()) ) );

    /// The expression @c p.apply_const(f) is equivalent to @c f(p.get()).
    template <typename Func>
    void apply_const( Func f ) const; // noexcept( noexcept( f(get()) ) );

    /////////////////////
    // other operators //
    /////////////////////

    /// Returns the contained pointer. A deep copy may be performed. May Throw!
    T * operator->();

    /// Returns the contained pointer propagating constness.
    const T * operator->() const noexcept;

    /// Cast for use in conditional statements
    operator bool() const noexcept;

    //////////////////////
    // helper functions //
    //////////////////////

    // Helper function for implementing make_cow.
    /* Creates a Y object with constructor argments args and returns a
        cow pointer to it. */
    template <typename Y, typename ...Args>
    static cow_ptr make( Args&&...args );

private:
    // This helper function makes a copy of the internal object if the
    // reference count is 2 or greater.
    void moo();

    ////////////////////
    // helper classes //
    ////////////////////

    // Class managing the reference count.
    /* The responsibility of this class is to manage the reference counter and
        to delete itself when the reference count drops to zero. */
    class abstract_counter
    {
    public:
        // Constructs a reference counter.
        // Postcondition: The reference count is zero.
        abstract_counter() noexcept;

        // Precondition. The reference count must be zero.
        virtual ~abstract_counter() noexcept;

        // Creates a new copy of the wrapping cow ptr whose reference count
        // is 1.
        virtual cow_ptr clone() const = 0;

        // Increments the reference count.
        void acquire() noexcept;

        // Decrements the reference count. Deletes the this object, if the
        // reference count drops to zero.
        void release() noexcept;

        // Returns true, iff the reference count is at most 1.
        bool unique() const noexcept;

    private:
        // internal reference counter.
        std::atomic<size_t> n_count;
    };

    // Reference counter containing owned pointer.
    /* At destruction (which occurs when the reference count drops to zero)
        the contained pointer is deleted using the deleter. */
    template <typename Y, typename D, typename C>
    class concrete_counter : public abstract_counter
    {
    public:
        // Takes ownership of @c p.
        concrete_counter( Y * p, D && deleter, C && cloner );

        // Calls @c deleter(p) which are given at construction.
        virtual ~concrete_counter() noexcept;

        // Copies @c *p using @c cloner.
        virtual cow_ptr clone() const;

    private:
        Y * p;
        D deleter;
        C cloner;
    };

    // Reference counter wrapping an @c Y object value.
    template <typename Y>
    class wrapping_counter : public abstract_counter
    {
    public:
        // Forwards @c args to the constructor of the contained @c Y object.
        template <typename ...Args>
        wrapping_counter( Args&&...args );

        // empty implementation.
        virtual ~wrapping_counter() noexcept;

        // Constructs a copy wrapped in a cow ptr using make_cow with the
        // copy constructor of @c Y.
        virtual cow_ptr clone() const;

        // Returns a @c T pointer to the wrapped object.
        T * get_ptr() noexcept;

    private:
        Y y;
    };

    //////////////////
    // data members //
    //////////////////

    T * px;
    abstract_counter * pn;
};

/// Returns a reference to the object pointed to propagating constness.
template <typename T>
const T & operator*( const cow_ptr<T> & p ) noexcept;

/// Exception-safe, efficient way to construct a cow pointer.
/** The code
      @code
        auto p = make_cow<Base,Derived>( ... );
      @endcode
    is equivalent to
      @code
        cow_ptr<Base> p( new Derived(...) );
      @endcode */
template <typename T, typename Y = T, typename ...Args>
cow_ptr<T> make_cow( Args&&...args );

/// Macro to simplify applying functors to the internal pointer of a
/// cow pointer.
/** Instead of
      @code
        ptr.apply( [&](ClassName * ptr) { ... do something with ptr ... } );
      @endcode
    you can write
      @code
        COW_APPLY(ptr) { ... do something with ptr ... };
      @endcode
    and thereby avoiding to write the unnecessary lambda hullabaloo which
    includes repeating the typename of the wrapped pointer. This also works
    properly for constant cow pointers. In this case the const member function
    @c apply() is called.
  @note Do not forget the semicolon at the end of the line. The template
    argument must be an identifier. */
#define COW_APPLY(ptr) ptr &= [&](decltype(ptr.operator->()) ptr)
//                         ^^ this operator is customized for this purpose

/// The const version of @c COW_APPLY.
/** It is guaranteed that no internal copy is made, since the const version
    of the @c apply() member function is called. */
#define COW_APPLY_CONST(ptr) make_const_ref(ptr) &= [&](decltype(ptr.get()) ptr)
//                           ^^ this template function is defined later.

// Helper operator for the implementation of COW_APPLY
template <typename T, typename Func>
void operator&=( cow_ptr<T> & p, Func && f );

// Helper operator for the implementation of COW_APPLY
template <typename T, typename Func>
void operator&=( const cow_ptr<T> & p, Func && f );

// Helper template function for the implementation of COW_APPLY_CONST
template <typename T>
const T & make_const_ref( T & ref );

/// Non-member version of swap.
template <typename T>
void swap( cow_ptr<T> & lhs, cow_ptr<T> & rhs ) noexcept;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *     IMPLEMENTATION OF cow_ptr's MEMBER FUNCTIONS                          *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

template <typename T>
cow_ptr<T>::cow_ptr() noexcept
    : px(nullptr)
    , pn(nullptr)
{
}


template <typename T>
cow_ptr<T>::cow_ptr( const cow_ptr & other ) noexcept
    : px(other.px)
    , pn(other.pn)
{
    pn->acquire();
}


template <typename T>
cow_ptr<T>::cow_ptr( cow_ptr && other ) noexcept
    : px(std::move(other.px))
    , pn(std::move(other.pn))
{
    other.px = nullptr;
    other.pn = nullptr;
}


template <typename T>
template <typename Y, typename D, typename C>
cow_ptr<T>::cow_ptr( Y * p, D deleter, C cloner )
    : px(p)
    , pn(new concrete_counter<Y,D,C>( p,std::move(deleter),std::move(cloner) ) )
{
    static_assert( noexcept(deleter(p)), "The deleter must not throw." );
    pn->acquire();
}


template <typename T>
cow_ptr<T>::~cow_ptr() noexcept
{
    if ( pn != nullptr )
        pn->release();
}


template <typename T>
void cow_ptr<T>::swap( cow_ptr & other ) noexcept
{
    std::swap( px, other.px );
    std::swap( pn, other.pn );
}


template <typename T>
cow_ptr<T> & cow_ptr<T>::operator=( cow_ptr other ) noexcept
{
    swap( other );
    return *this;
}


template <typename T>
const T * cow_ptr<T>::get() const noexcept
{
    return px;
}


template <typename T>
template <typename Func>
void cow_ptr<T>::apply( Func f )
{
    moo();
    f( px );
}


template <typename T>
template <typename Func>
void cow_ptr<T>::apply( Func f ) const // noexcept( noexcept( f(get()) ) )
{
    apply_const( std::forward<Func>(f) );
}


template <typename T>
template <typename Func>
void cow_ptr<T>::apply_const( Func f ) const // noexcept( noexcept( f(get()) ) )
{
    f( get() );
}


template <typename T>
T * cow_ptr<T>::operator->()
{
    assert( *this );
    moo();
    return px;
}


template <typename T>
const T * cow_ptr<T>::operator->() const noexcept
{
    assert( *this );
    return px;
}


template <typename T>
cow_ptr<T>::operator bool() const noexcept
{
    return px != nullptr;
}


template <typename T>
template <typename Y, typename ...Args>
cow_ptr<T> cow_ptr<T>::make( Args&&...args )
{
    auto pn = new typename cow_ptr<T>::template wrapping_counter<Y>(
                std::forward<Args>(args)... );
    pn->acquire();
    cow_ptr p;
    p.px = pn->get_ptr();
    p.pn = pn;

    return p;
}


template <typename T>
void cow_ptr<T>::moo()
{
    if ( px != nullptr && !pn->unique() )
        pn->clone().swap(*this);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *     IMPLEMENTATION OF CLASS cow_ptr<T>::abstract_counter                  *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

template <typename T>
cow_ptr<T>::abstract_counter::abstract_counter() noexcept
    : n_count(0)
{
}


template <typename T>
cow_ptr<T>::abstract_counter::~abstract_counter() noexcept
{
    assert( n_count == 0 );
}


template <typename T>
void cow_ptr<T>::abstract_counter::acquire() noexcept
{
    ++n_count;
}


template <typename T>
void cow_ptr<T>::abstract_counter::release() noexcept
{
    if ( --n_count == 0 )
        delete this;
}


template <typename T>
bool cow_ptr<T>::abstract_counter::unique() const noexcept
{
    return n_count.load() <= 1;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *     IMPLEMENTATION OF CLASS cow_ptr<T>::concrete_counter<Y,D,C>           *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

template <typename T>
template <typename Y, typename D, typename C>
cow_ptr<T>::concrete_counter<Y,D,C>::concrete_counter(
        Y * p, D && deleter, C && cloner )
// && does not indicate universal references (as defined by Scott Meyers) here,
// since D and C are already bound to some types. Hence deleter and cloner are
// genuine rvalue references and can be safely moved from.
    : p(p)
    , deleter( std::move(deleter) )
    , cloner( std::move(cloner) )
{
}


template <typename T>
template <typename Y, typename D, typename C>
cow_ptr<T>::concrete_counter<Y,D,C>::~concrete_counter() noexcept
{
    deleter(p);
}


template <typename T>
template <typename Y, typename D, typename C>
cow_ptr<T> cow_ptr<T>::concrete_counter<Y,D,C>::clone() const
{
    return cow_ptr<T>(
        p != nullptr ? cloner(const_cast<const Y *>(p)) : nullptr,
        deleter, cloner );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *     IMPLEMENTATION OF CLASS cow_ptr<T>::wrapping_counter<Y>               *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

template <typename T>
template <typename Y>
template <typename ...Args>
cow_ptr<T>::wrapping_counter<Y>::wrapping_counter( Args&&...args )
    : y( std::forward<Args>(args)... )
{
}


template <typename T>
template <typename Y>
cow_ptr<T>::wrapping_counter<Y>::~wrapping_counter() noexcept
{
}


template <typename T>
template <typename Y>
cow_ptr<T> cow_ptr<T>::wrapping_counter<Y>::clone() const
{
    return make_cow<T,Y>(y);
}


template <typename T>
template <typename Y>
T * cow_ptr<T>::wrapping_counter<Y>::get_ptr() noexcept
{
    return &y;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *     IMPLEMENTATION OF cow_ptr's NON-MEMBER FUNCTIONS                      *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

template <typename T>
const T & operator*( const cow_ptr<T> & p ) noexcept
{
    assert( p );
    return *p.get();
}


template <typename T>
T & operator*( cow_ptr<T> & p )
{
    assert( p );
    return *p.get();
}


template <typename T, typename Y, typename ...Args>
cow_ptr<T> make_cow( Args&&...args )
{
    return cow_ptr<T>::template make<Y>( std::forward<Args>(args)... );
}


template <typename T, typename Func>
void operator&=( cow_ptr<T> & p, Func && f )
{
    p.apply( std::forward<Func>(f) );
}


template <typename T, typename Func>
void operator&=( const cow_ptr<T> & p, Func && f )
{
    p.apply( std::forward<Func>(f) );
}


template <typename T>
const T & make_const_ref( T & ref )
{
    return ref;
}


template <typename T>
void swap( cow_ptr<T> & lhs, cow_ptr<T> & rhs ) noexcept
{
    lhs.swap( rhs );
}

} // namespace cow

#endif // COW_PTR_H
