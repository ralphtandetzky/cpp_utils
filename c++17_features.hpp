/** @file This file defines features available from C++17 onwards.
 *
 * The features are defined in the cu namespace instead of the std namespace.
 * When moving to C++17 just remove this header from the project,
 * in order to keep you project clean. This should require only a bit of
 * refactoring by replacing the respective @c cu:: by @c std::.
 *
 * @author Ralph Tandetzky
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#ifdef __has_include                           // Check if __has_include is present
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>
#  elif __has_include(<boost/optional.hpp>)    // Try with an external library
#    include <boost/optional.hpp>
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#else
#  error "Compiler does not support __has_include"
#endif

namespace cu
{

//using std::experimental::optional;
// #include <optional>
//
// std::optional<T>

#if __has_include(<optional>)                // Check for a standard library
using std::optional;
#elif __has_include(<experimental/optional>) // Check for an experimental version
using std::experimental::optional;
#elif __has_include(<boost/optional.hpp>)    // Try with an external library
using boost::optional;
#endif

// #include <iterator>
//
// std::size()

template <typename Container>
constexpr auto size( const Container& container ) -> decltype(container.size())
{
    return container.size();
}


template <typename T, std::size_t N>
constexpr std::size_t size( const T (&)[N] ) noexcept
{
    return N;
}


// #include <functional>
//
// std::invoke()

namespace detail {
  template <typename T>
  struct is_reference_wrapper : std::false_type {};

  template <typename U>
  struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

  template <typename Base, typename T, typename Derived, typename... Args>
  auto INVOKE(T Base::*pmf, Derived&& ref, Args&&... args)
      noexcept(noexcept((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)))
   -> typename std::enable_if<std::is_function<T>::value &&
                              std::is_base_of<Base, typename std::decay<Derived>::type>::value,
      decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))>::type
  {
        return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
  }

  template <typename Base, typename T, typename RefWrap, typename... Args>
  auto INVOKE(T Base::*pmf, RefWrap&& ref, Args&&... args)
      noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
   -> typename std::enable_if<std::is_function<T>::value &&
                              is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
      decltype((ref.get().*pmf)(std::forward<Args>(args)...))>::type
  {
        return (ref.get().*pmf)(std::forward<Args>(args)...);
  }

  template <typename Base, typename T, typename Pointer, typename... Args>
  auto INVOKE(T Base::*pmf, Pointer&& ptr, Args&&... args)
      noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
   -> typename std::enable_if<std::is_function<T>::value &&
                              !is_reference_wrapper<typename std::decay<Pointer>::type>::value &&
                              !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
      decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))>::type
  {
        return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
  }

  template <typename Base, typename T, typename Derived>
  auto INVOKE(T Base::*pmd, Derived&& ref)
      noexcept(noexcept(std::forward<Derived>(ref).*pmd))
   -> typename std::enable_if<!std::is_function<T>::value &&
                              std::is_base_of<Base, typename std::decay<Derived>::type>::value,
      decltype(std::forward<Derived>(ref).*pmd)>::type
  {
        return std::forward<Derived>(ref).*pmd;
  }

  template <typename Base, typename T, typename RefWrap>
  auto INVOKE(T Base::*pmd, RefWrap&& ref)
      noexcept(noexcept(ref.get().*pmd))
   -> typename std::enable_if<!std::is_function<T>::value &&
                              is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
      decltype(ref.get().*pmd)>::type
  {
        return ref.get().*pmd;
  }

  template <typename Base, typename T, typename Pointer>
  auto INVOKE(T Base::*pmd, Pointer&& ptr)
      noexcept(noexcept((*std::forward<Pointer>(ptr)).*pmd))
   -> typename std::enable_if<!std::is_function<T>::value &&
                              !is_reference_wrapper<typename std::decay<Pointer>::type>::value &&
                              !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
      decltype((*std::forward<Pointer>(ptr)).*pmd)>::type
  {
        return (*std::forward<Pointer>(ptr)).*pmd;
  }

  template <typename F, typename... Args>
  auto INVOKE(F&& f, Args&&... args)
      noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
   -> typename std::enable_if<!std::is_member_pointer<typename std::decay<F>::type>::value,
      decltype(std::forward<F>(f)(std::forward<Args>(args)...))>::type
  {
        return std::forward<F>(f)(std::forward<Args>(args)...);
  }
} // namespace detail

template< typename F, typename... ArgTypes >
auto invoke(F&& f, ArgTypes&&... args)
    // exception specification for QoI
    noexcept(noexcept(detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...)))
 -> decltype(detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...))
{
    return detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...);
}


// #include <tuple>
//
// std::apply()

#ifdef I
#define CU_OLD_I_DEF I
#undef I
#endif
namespace detail {
  template <typename F, typename Tuple, std::size_t... I>
  constexpr decltype(auto) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> )
  {
    return cu::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
  }
} // namespace detail
#ifdef CU_OLD_I_DEF
#define I CU_OLD_I_DEF
#undef CU_OLD_I_DEF
#endif

template <typename F, typename Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
  return detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
}

} // namespace cu
