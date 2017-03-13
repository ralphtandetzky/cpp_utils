#pragma once

#include <cstddef>

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
    Slice() = default;
    Slice( T * ptr_, std::size_t size_ )
        : ptr(ptr_)
        , size(size_)
    {}

    T * begin() const
    {
        return ptr;
    }

    T * end() const
    {
        return ptr + size;
    }

    T * ptr = nullptr;
    std::size_t size = 0;
};

template <typename T>
Slice<T> makeSlice( T * ptr, std::size_t size )
{
    return { ptr, size };
};


} // namespace cu
