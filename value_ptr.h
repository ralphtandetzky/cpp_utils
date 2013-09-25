/// @file
///
/// @author Ralph Tandetzky
/// @date 21 Aug 2013

#pragma once
#include <utility>
#include <memory>

template <typename T>
class value_ptr
{
public:
    ~value_ptr();
    value_ptr() noexcept;
    template <typename Y>
    value_ptr( std::unique_ptr<Y> other );
    value_ptr( const value_ptr &  other );
    value_ptr(       value_ptr && other ) noexcept;
    value_ptr & operator=( value_ptr other ) noexcept;
    void swap( value_ptr & other ) noexcept;

    T * get() noexcept;
    const T * get() const noexcept;
    T * operator->() noexcept;
    const T * operator->() const noexcept;
    T & operator*() noexcept;
    const T & operator*() const noexcept;
    operator bool() const noexcept;

    template <typename Y = T, typename...Args>
    static value_ptr make( Args&&...args );

private:
    struct HelperInterface
    {
        virtual ~HelperInterface() {}
        virtual HelperInterface * clone() const = 0;
        virtual T * get() noexcept = 0;
    };

    template <typename Y>
    struct IntrusiveHelper : HelperInterface
    {
        template <typename ...Args> IntrusiveHelper( Args&&...args )
            : data(std::forward<Args>(args)...) {}
        HelperInterface * clone() const override
        { return new IntrusiveHelper( data ); }
        Y * get() noexcept override { return &data; }
        Y data;
    };

    template <typename Y>
    struct PointingHelper : HelperInterface
    {
        PointingHelper( std::unique_ptr<Y> p ) : p(std::move(p)) {}
        HelperInterface * clone() const override
        { return new IntrusiveHelper<Y>( *p ); }
        Y * get() noexcept override { return p.get(); }
        std::unique_ptr<Y> p;
    };

    T * px;
    HelperInterface * ph;

    // takes ownership of ph.
    value_ptr( HelperInterface * ph ) noexcept;
};

template <typename T, typename Y = T, typename ...Args>
value_ptr<T> make_value( Args&&...args )
{
    return value_ptr<T>::template make<Y>( std::forward<Args>(args)... );
}

template <typename T>
value_ptr<T>::~value_ptr()
{
    delete ph;
}

template <typename T>
value_ptr<T>::value_ptr() noexcept
    : px(), ph()
{
}

template <typename T>
template <typename Y>
value_ptr<T>::value_ptr(std::unique_ptr<Y> other)
    : px(), ph()
{
    if ( other )
    {
        ph = new PointingHelper<Y>(std::move(other));
        px = ph->get();
    }
}

template <typename T>
value_ptr<T>::value_ptr( const value_ptr<T> & other )
    : px(), ph()
{
    if ( other.ph )
    {
        ph = other.ph->clone();
        px = ph->get();
    }
}

template <typename T>
value_ptr<T>::value_ptr( value_ptr<T> && other ) noexcept
    : px(std::move(other.px))
    , ph(std::move(other.ph))
{
    other.px = nullptr;
    other.ph = nullptr;
}

template <typename T>
value_ptr<T> & value_ptr<T>::operator=( value_ptr<T> other ) noexcept
{
    swap(other);
    return *this;
}

template <typename T>
void value_ptr<T>::swap( value_ptr<T> & other ) noexcept
{
    ::std::swap( px, other.px );
    ::std::swap( ph, other.ph );
}

template <typename T>
T * value_ptr<T>::get() noexcept
{
    return px;
}

template <typename T>
const T * value_ptr<T>::get() const noexcept
{
    return px;
}

template <typename T>
T * value_ptr<T>::operator->() noexcept
{
    return get();
}

template <typename T>
const T * value_ptr<T>::operator->() const noexcept
{
    return get();
}

template <typename T>
T & value_ptr<T>::operator*() noexcept
{
    return *get();
}

template <typename T>
const T & value_ptr<T>::operator*() const noexcept
{
    return *get();
}

template <typename T>
value_ptr<T>::operator bool() const noexcept
{
    return px!=nullptr;
}

template <typename T>
template <typename Y, typename ...Args>
value_ptr<T> value_ptr<T>::make( Args&&...args )
{
    return value_ptr( new IntrusiveHelper<Y>(std::forward<Args>(args)...) );
}

template <typename T>
value_ptr<T>::value_ptr( value_ptr<T>::HelperInterface * ph ) noexcept
    : px( ph ? ph->get() : nullptr )
    , ph(ph)
{
}
