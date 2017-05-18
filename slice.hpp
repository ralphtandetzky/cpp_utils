#pragma once

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <stdexcept>

namespace cu
{

/// A view into an array of contiguous elements.
///
/// It contains a pointer and a size as information and supports reference
/// semantics to the underlying storage. It is similiar to std::string_view
/// or std::array_view.
template <typename T>
struct Slice
{
    Slice() noexcept = default;

    Slice( T * ptr, std::size_t size ) noexcept
        : ptr_ (ptr )
        , size_(size)
    {}

    Slice( T * first, T * last ) noexcept
      : Slice( first, last-first )
    {}

    Slice( std::initializer_list<T> list )
      : Slice( list.begin(), list.size() )
    {}

    template <typename Container,
              typename std::enable_if<
                std::is_same<T*,typename Container::pointer>::value,int>::type = 0>
    Slice( Container & container ) noexcept
      : Slice( container.data(), container.size() )
    {}

    template <typename Container,
              typename std::enable_if<
                std::is_same<T*,typename Container::const_pointer>::value,int>::type = 0>
    Slice( const Container & container ) noexcept
      : Slice( container.data(), container.size() )
    {}

    template <std::size_t N>
    Slice( T(&arr)[N] ) noexcept
      : Slice( arr, N )
    {}

    T * begin() const noexcept
    {
        return ptr_;
    }

    T * end() const noexcept
    {
        return ptr_ + size_;
    }

    T * data() const noexcept
    {
      return ptr_;
    }

    std::size_t size() const noexcept
    {
      return size_;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    void swap( Slice & other ) noexcept
    {
      Slice tmp = other;
      other = *this;
      *this = tmp;
    }

    T & operator[]( std::size_t index ) const noexcept
    {
      return ptr_[index];
    }

    T & at( std::size_t index ) const
    {
      if ( index >= size_ )
        throw std::out_of_range("cu::Slice access out of range.");
    }

    T & front() const noexcept
    {
      return *ptr_;
    }

    T & back() const noexcept
    {
      return ptr_[size_-1];
    }

    Slice withoutHead() const noexcept
    {
      return { ptr_+1, size_-1 };
    }

    Slice withoutTail() const noexcept
    {
      return { ptr_, size_-1 };
    }

private:
    T * ptr_ = nullptr;
    std::size_t size_ = 0;
};


template <typename T>
Slice<T> makeSlice( T * ptr, std::size_t size )
{
    return { ptr, size };
};

template <typename T>
Slice<T> makeSlice( T * first, T * last )
{
    return { first, last };
};

template <typename Container>
Slice<typename Container::value_type> makeSlice( Container & container )
{
  return { container };
}

template <typename Container>
Slice<const typename Container::value_type> makeSlice( const Container & container )
{
  return { container };
}

template <typename T, std::size_t N>
Slice<T> makeSlice( T(&arr)[N] )
{
  return { arr };
}

template <typename T>
void swap( Slice<T> & lhs, Slice<T> rhs )
{
  lhs.swap( rhs );
}

} // namespace cu
