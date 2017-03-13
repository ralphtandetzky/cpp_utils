// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

#pragma once

#include <atomic>
#include <cassert>
#include <memory>

namespace cu
{

class DefaultCloner
{
public:
  template <typename T>
  std::unique_ptr<T> operator()( const T & item ) const
  {
    return doClone( item, Rank2{} );
  }

private:
  struct Rank0 {};
  struct Rank1 : Rank0 {};
  struct Rank2 : Rank1 {};

  // Prefer member clone.
  template <typename T>
  static auto doClone( const T & item, Rank2 )
    -> decltype((void)item.clone(),std::unique_ptr<T>{})
  {
    return item.clone();
  }

  // Non-member clone is second choice.
  template <typename T>
  static auto doClone( const T & item, Rank1 )
    -> decltype((void)clone(item),std::unique_ptr<T>{})
  {
    return clone( item );
  }

  // Last resort: Clone manually.
  template <typename T>
  static auto doClone( const T & item, Rank0 )
  {
    return std::make_unique<T>( item );
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
        cow_ptr<X> a = std::make_unique<X>();
        cow_ptr<X> b = a; // b points to the same object as a internally.
        cow_ptr<X> c;
        c = b;     // a, b and c all point to the same object internally.
        b.modify( [](auto){ ... } ); // A new copy of b is created and then
                                     // the lambda is called. a and c still
                                     // point to the same object internally.
        b.modify( [](auto){ ... } ); // No extra copying is performed here.
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
      * It implements copy-on-write. Here is a simple use case:
        For classes with members whose copy-operations are expensive and/or
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
                size_t nRows, nCols; // can be modified without deep copy.
                cow_ptr<std::vector<float>> data; // copy-on-write
            }
          @endcode
        In such classes the default version of copy and move construction
        will usually work just fine.
      * You can add cloning to a class hierarchy from the outside. With
          @code
            cow_ptr<Base> a = std::make_unique<Derived1>;
            cow_ptr<Base> b = std::make_unique<Derived2>;
            cow_ptr<Base> c;
            c = a; // performs a shallow copy.
            c.modify( [](auto){ ...} ); // makes a deep copy of a as a Derived1
                                        // class. There is no slicing involved.
          @endcode
        you copy @c Base objects polymorphically. The class @c Base can even
        be abstract here. It is only required that @c Derived1 and
        @c Derived2 be @c CopyConstructible.
      * You can create arrays with elements that retain polymorphic behavior
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
            raw->mutate(); // modifies the guts of both p and q
          @endcode
        would break the copy-on-write semantics of the cow pointer.
    In order to make the user interface of the class easy to use correctly and
    hard to use incorrectly, a member template function @c modify() is given.
    This function takes a functor as argument which takes a @c T* argument.
    It makes an internal copy of the object pointed to, if the reference count
    is at least 2 and then it calls the functor with the stored @c T* pointer.
    Hence the code above would be written like this:
      @code
        cow_ptr<T> p = std::make_unique<T>();
        cow_ptr<T> q = p;
        p.modify( []( auto * raw ){ raw->mutate(); } ); // performes deep copy first.
      @endcode
    It is still possible to let pointers escape and thereby break the
    copy-on-write semantics as mentioned above, but it is harder. Using the

    Similar to @c std::make_shared and @c std::make_unique there is an
    equivalent @c make_cow which makes code faster, more exception-safe
    and less repetitive. You can write
      @code
        auto p = make_cow<Base,Derived>( ... );
      @endcode
    instead of
      @code
        cow_ptr<Base> p = std::make_unique<Derived>(...);
      @endcode
    which has the advantage that only one allocation with @c new is performed
    instead of two. If @c Base and @c Derived are the same, then you can omit
    the second template argument to @c make_cow, which spares you of repeating
    the type. (To see why this is good practice in order to obtain
    exception-safe code see http://herbsutter.com/gotw/_102/)

    Requirements on @c T: The only requirement is that @c T fulfills one of the
    following three conditions:
      * @c T is @c CopyConstructible and @c operator new() is be accessible.
        (If a @c default_cloner is used.)

    Exception-safety: Construction to a non-empty state and modifying the
    pointee may throw. All other operations (including copy construction
    and copy assignment) do not throw!
    This may change the exception guarantees of a class, if cow pointers are
    used as pimpl pointers.
    @c cow_ptr<T> is exception-neutral, which means that all raised exceptions
    propagate out from cow_ptr<T> without being handled or swallowed.
    Furthermore, @c cow_ptr<T> itself does not throw, but only called code may
    throw.

    Thread-safety: Here we speak of 'reads' as const operations and 'writes'
    as non-const operations. Each cow pointer object can be safely read by
    multiple threads at the same time as long as no thread is writing to the
    object at the same time. It is safe to use cow pointers in one thread as
    long as all copies of that cow pointer and any references to the pointed
    to object stay within that thread.
    If the clone operation of a cow pointer object is thread-safe,
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
  /// Initialize as @c nullptr.
  cow_ptr() noexcept = default;

  /// Move constructor.
  cow_ptr( cow_ptr && other ) noexcept { swap( other ); }

  /// Copy constructor.
  cow_ptr( const cow_ptr & other ) noexcept = default;

  /// Move assignment.
  cow_ptr & operator=( cow_ptr && other ) noexcept { swap( other ); return *this; }

  /// Copy assignment.
  cow_ptr & operator=( const cow_ptr & other ) noexcept = default;

  /// Initialize as @c nullptr.
  cow_ptr( std::nullptr_t ) noexcept {}

  /// Construct from unique_ptr.
  template <typename U,
            typename D,
            typename C = DefaultCloner>
  cow_ptr( std::unique_ptr<U,D> p,
           C cloner = DefaultCloner() )
    : px( p.get() )
    , pn( RefCounterPtr( std::move(p), std::move(cloner) ) )
  {}

  /// Emplace-construct pointee.
  template <typename U,
            typename ...Args>
  static cow_ptr<T> make( Args &&... args )
  {
    cow_ptr<T> result;
    result.pn = RefCounterPtr( EmplaceTag<U>{}, std::forward<Args>(args)... );
    result.px = result.pn.get();
    return result;
  }

  /// No-fail swap.
  void swap( cow_ptr & other ) noexcept
  {
    std::swap( px, other.px );
    pn.swap( other.pn );
  }

  /// Tells if the reference count is 1.
  ///
  /// @note There is no non-const overload on purpose in order to avoid
  /// accidental deep copies.
  /// Use @c modify() if you want to call non-const methods on the pointee.
  bool unique() const noexcept
  {
    return pn.unique();
  }

  /// Const-correct raw pointer retrieval.
  ///
  /// @note There is no non-const overload on purpose in order to avoid
  /// accidental deep copies.
  /// Use @c modify() if you want to call non-const methods on the pointee.
  const T * get() const noexcept
  {
    return px;
  }

  /// @c operator-> for const access.
  ///
  /// @note There is no non-const overload on purpose in order to avoid
  /// accidental deep copies.
  /// Use @c modify() if you want to call non-const methods on the pointee.
  const T * operator->() const noexcept
  {
    assert( px );
    return px;
  }

  /// @c operator* for const access.
  ///
  /// @note There is no non-const overload on purpose in order to avoid
  /// accidental deep copies.
  /// Use @c modify() if you want to call non-const methods on the pointee.
  const T & operator*() const noexcept
  {
    assert( px );
    return *px;
  }

  /// Tells if the pointer is not null.
  operator bool() const noexcept
  {
    return px;
  }

  /// Write access to the pointee.
  ///
  /// @param f A functor that takes a @c T* as parameter.
  /// The return value of the functor will be returned to the caller.
  template <typename F>
  decltype(auto) modify( F && f )
  {
    // Perform copy, if not unique.
    if ( !unique() )
      pn.clone().swap(*this);

    // Apply functor.
    return f( px );
  }

private:
  template <typename U>
  struct EmplaceTag {};

  /// Abstract reference counter base class.
  class RefCounterBase
  {
  public:
    // The reference count will be initialized to 1.
    RefCounterBase() = default;
    virtual ~RefCounterBase() = default;
    RefCounterBase( const RefCounterBase & ) = delete;
    RefCounterBase & operator=( const RefCounterBase & ) = delete;
    virtual T * get() noexcept = 0;
    virtual cow_ptr<T> clone() const = 0;

    bool unique() const noexcept
    {
      return count.load( std::memory_order_relaxed ) == 0;
    }

    void increment() noexcept
    {
      count.fetch_add( 1, std::memory_order_relaxed );
    }

    int decrement() noexcept
    {
      return count.fetch_sub( 1, std::memory_order_acq_rel );
    }

  private:
    // This field holds the reference count - 1.
    std::atomic<std::size_t> count{0}; // initial reference count: 1!
  };

  /// Smart pointer for ref count pointees.
  ///
  /// This class automatically keeps track of the reference count of
  /// the pointee. The pointee is deleted when the last pointer to it
  /// is destroyed.
  class RefCounterPtr
  {
  public:
    ~RefCounterPtr()
    {
      if ( p != nullptr && p->decrement() == 0 )
        delete p;
    }

    RefCounterPtr() = default;
    RefCounterPtr( const RefCounterPtr & other ) noexcept
      : p( other.p )
    {
      if ( p )
        p->increment();
    }

    RefCounterPtr( RefCounterPtr && other ) noexcept
    {
      swap( other );
    }

    RefCounterPtr & operator=( RefCounterPtr other ) noexcept
    {
      swap( other );
      return *this;
    }

    template <typename U,
              typename D,
              typename C>
    RefCounterPtr( std::unique_ptr<U,D> p_, C cloner )
    {
      if ( !p_ )
        return;
      p = new ExternalRefCounter<U,D,C>( std::move(p_), std::move(cloner) );
    }

    template <typename U,
              typename ...Args>
    RefCounterPtr( EmplaceTag<U>, Args &&... args )
      : p( new IntrusiveRefCounter<U>( std::forward<Args>(args)... ) )
    {}

    void swap( RefCounterPtr & other ) noexcept
    {
      std::swap( p, other.p );
    }

    bool unique() const noexcept
    {
      return p == nullptr || p->unique();
    }

    T * get() noexcept
    {
      return p->get();
    }

    cow_ptr<T> clone() const
    {
      return p->clone();
    }

  private:
    RefCounterBase * p = nullptr;
  };

  template <typename U,
            typename D,
            typename C>
  class ExternalRefCounter final
      : public RefCounterBase
  {
  public:
    ExternalRefCounter(
        std::unique_ptr<U,D> px_,
        C cloner_ )
      : px( std::move(px_) )
      , cloner( std::move(cloner_) )
    {}

    virtual T * get() noexcept
    {
      return px.get();
    }

    virtual cow_ptr<T> clone() const
    {
      return cow_ptr<T>( cloner( *px ), cloner );
    }

  private:
    std::unique_ptr<U,D> px;
    C cloner;
  };

  template <typename U>
  class IntrusiveRefCounter final
      : public RefCounterBase
  {
  public:
    template <typename ...Args>
    IntrusiveRefCounter( Args &&... args )
      : item( std::forward<Args>(args)... )
    {}

    virtual T * get() noexcept
    {
      return &item;
    }

    virtual cow_ptr<T> clone() const
    {
      return make<U>( item );
    }

  private:
    U item;
  };

  T * px = nullptr;
  RefCounterPtr pn;
};


/// Emplace-construct pointee just like @c std::make_shared().
template <typename T,
          typename U = T,
          typename ...Args>
cow_ptr<T> make_cow( Args &&... args )
{
  return cow_ptr<T>::template make<U>( std::forward<Args>(args)... );
}

template <typename T>
cow_ptr<T> to_cow_ptr( T data )
{
  return cu::make_cow<T>( std::move(data) );
}

/// No-fail swap.
template <typename T>
void swap( cow_ptr<T> & lhs, cow_ptr<T> & rhs ) noexcept
{
  lhs.swap(rhs);
}

template <typename T>
bool operator==( const cow_ptr<T> & lhs, std::nullptr_t )
{
  return lhs.get() == nullptr;
}

} // namespace cu
