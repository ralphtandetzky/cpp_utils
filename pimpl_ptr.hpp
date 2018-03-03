/** @file Defines the @c pimpl_ptr class.
 * @author Ralph Tandetzky
 * @date 16 Jun 2016
 */

#pragma once

#include <memory>

namespace cu
{

/// This smart pointer class is designed to be used for the pimpl idiom.
///
/// @see https://en.wikibooks.org/wiki/C%2B%2B_Programming/Idioms#Pointer_To_Implementation_.28pImpl.29
/// for more information on the pimpl idiom.
///
/// This smart pointer type behaves like a @c std::unique_ptr, except
///   * A living pimpl_ptr never points to null.
///     This avoids often needless validity checks.
///   * The template constructor forwards all arguments to the pointee
///     constructor.
///   * constness is propagated. Hence, when accessing a pointee through
///     a constant @c pimpl_ptr, then the pointee is also constant.
///   * @c pimpl_ptr is not movable, but only swappable.
///   * The destructor of @c pimpl_ptr does not need to see the definition
///     of the template type @c T, due to type erasure using a costum deleter.
template <typename T>
class pimpl_ptr
{
public:
  template <typename ...Args>
  explicit pimpl_ptr( Args &&... args )
    : p( new T( std::forward<Args>(args)... ), [](T*p){ delete p; } )
  {}

  pimpl_ptr(       pimpl_ptr && ) = delete;
  pimpl_ptr( const pimpl_ptr &  ) = delete;
  pimpl_ptr & operator=( pimpl_ptr ) = delete;
  void swap( pimpl_ptr & other ) noexcept { p.swap( other.p ); }

        T & operator* ()       { return *p; }
  const T & operator* () const { return *p; }
        T * operator->()       { return p.get(); }
  const T * operator->() const { return p.get(); }
        T * get       ()       { return p.get(); }
  const T * get       () const { return p.get(); }

private:
  std::unique_ptr<T,void(*)(T*)> p;
};

} // namespace cu
