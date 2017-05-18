#pragma once

#include "monitor.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

namespace cu
{

class RegionAllocatorImpl
{
public:
  RegionAllocatorImpl()
    : storagePtr( std::make_shared<Storage>() )
  {}

  void * allocate( std::size_t n, std::size_t alignment )
  {
    assert( ( alignment & (alignment-1) ) == 0 &&
            "The alignment must be a power of two." );
    auto & storage = *storagePtr;
    if ( n >= minBigChunkSize )
    {
      auto p = makeMallocPtr( n );
      const auto result = p.get();
      assert( result == align( result, alignment ) );
      storage.bigChunks( [&]( auto & bigChunks )
      {
        bigChunks.push_back( std::move(p) );
      } );
      return result;
    }

    const auto p = updateRange( storage.range, n, alignment );
    if ( p )
      return p;

    return storage.regions( [&]( auto & regions ) -> void*
    {
      const auto p = updateRange( storage.range, n, alignment );
      if ( p )
        return p;
      auto region = makeMallocPtr( regionSize );
      const auto result = region.get();
      assert( result == align( result, alignment ) );
      storage.range = { result, result + regionSize };
      regions.push_back( std::move( region ) );
      return result;
    } );
  }

  bool operator==( const RegionAllocatorImpl & other ) const
  {
    return storagePtr.get() == other.storagePtr.get();
  }

private:
  enum {
    regionSize      = 8192,
    minBigChunkSize = 2048,
  };

  struct Range
  {
    char * begin;
    char * end;
  };

  struct FreeDeleter
  {
    void operator()( void * p )
    {
      std::free( p );
    }
  };

  using MallocPtr = std::unique_ptr<char[],FreeDeleter>;

  static MallocPtr makeMallocPtr( std::size_t n )
  {
    const auto p = static_cast<char*>( malloc(n) );
    if ( !p )
      throw std::bad_alloc{};
    return MallocPtr( p );
  }

  static char * align( char * p, std::size_t alignment )
  {
    return reinterpret_cast<char*>(
           reinterpret_cast<std::size_t>( p + (alignment-1) ) & -alignment );
  }

  static Range chopRange(
      const Range & range,
      std::size_t n,
      std::size_t alignment )
  {
    return { align( range.begin, alignment ) + n, range.end };
  }

  /// Atomically chops at least @c n bytes from the front of the range and
  /// returns an aligned pointer @c p that points into the original range,
  /// such that @c [p,p+n] does not overlap with the resulting @c range.
  /// If that is not possible, then a @c nullptr will be returned.
  static void * updateRange(
      std::atomic<Range> & range,
      std::size_t n,
      std::size_t alignment )
  {
    auto oldRange = range.load();
    for ( ;; )
    {
      const auto choppedRange = chopRange( oldRange, n, alignment );
      if ( choppedRange.begin > choppedRange.end )
        return nullptr; // failure
      if ( range.compare_exchange_weak( oldRange, choppedRange ) )
        return align( oldRange.begin, alignment ); // success
    }
  }

  struct Storage
  {
    Storage()
    {
      const auto reserver = []( auto & v ){ v.reserve(64/sizeof(MallocPtr)); };
      regions  ( reserver );
      bigChunks( reserver );
    }

    cu::Monitor<std::vector<MallocPtr>> regions;
    cu::Monitor<std::vector<MallocPtr>> bigChunks;
    std::atomic<Range> range{};
  };

  std::shared_ptr<Storage> storagePtr;
};


template <typename T>
class RegionAllocator {
public:
  using value_type = T;

  RegionAllocator( const RegionAllocator & ) = default;
  RegionAllocator & operator=( const RegionAllocator & ) = default;

  template <typename U>
  RegionAllocator( const RegionAllocator<U> & other )
    : impl( std::move( other.impl ) )
  {}

  T* allocate( std::size_t n )
  {
    return static_cast<T*>( impl.allocate( n * sizeof(T), alignof(T) ) );
  }

  void deallocate( T *, std::size_t )
  {
    // NOOP. Memory is freed in one go when the last copy of @c impl
    // is destroyed.
  }

  template <typename U>
  std::enable_if_t<!std::is_same<U,void>::value,bool>
    operator==( const RegionAllocator<U> & other )
  {
    return impl == other.impl;
  }

  template <typename U>
  std::enable_if_t<!std::is_same<U,void>::value,bool>
    operator!=( const RegionAllocator<U> & other )
  {
    return impl != other.impl;
  }

  template <typename U>
  friend class RegionAllocator;

private:
  RegionAllocatorImpl impl;
};

template <>
class RegionAllocator<void>
{
public:
  using value_type = void;

  RegionAllocator() = default;
  RegionAllocator( const RegionAllocator & ) = default;
  RegionAllocator & operator=( const RegionAllocator & ) = default;

  template <typename U>
  friend class RegionAllocator;

private:
  RegionAllocatorImpl impl;
};

template <typename T>
using RegionVector = std::vector<T, RegionAllocator<T>>;

} // namespace cu
