// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

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

namespace cu
{

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
   -> std::enable_if_t<std::is_function<T>::value &&
                       std::is_base_of<Base, std::decay_t<Derived>>::value,
      decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))>
  {
        return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
  }

  template <typename Base, typename T, typename RefWrap, typename... Args>
  auto INVOKE(T Base::*pmf, RefWrap&& ref, Args&&... args)
      noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
   -> std::enable_if_t<std::is_function<T>::value &&
                       is_reference_wrapper<std::decay_t<RefWrap>>::value,
      decltype((ref.get().*pmf)(std::forward<Args>(args)...))>

  {
        return (ref.get().*pmf)(std::forward<Args>(args)...);
  }

  template <typename Base, typename T, typename Pointer, typename... Args>
  auto INVOKE(T Base::*pmf, Pointer&& ptr, Args&&... args)
      noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
   -> std::enable_if_t<std::is_function<T>::value &&
                       !is_reference_wrapper<std::decay_t<Pointer>>::value &&
                       !std::is_base_of<Base, std::decay_t<Pointer>>::value,
      decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))>
  {
        return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
  }

  template <typename Base, typename T, typename Derived>
  auto INVOKE(T Base::*pmd, Derived&& ref)
      noexcept(noexcept(std::forward<Derived>(ref).*pmd))
   -> std::enable_if_t<!std::is_function<T>::value &&
                       std::is_base_of<Base, std::decay_t<Derived>>::value,
      decltype(std::forward<Derived>(ref).*pmd)>
  {
        return std::forward<Derived>(ref).*pmd;
  }

  template <typename Base, typename T, typename RefWrap>
  auto INVOKE(T Base::*pmd, RefWrap&& ref)
      noexcept(noexcept(ref.get().*pmd))
   -> std::enable_if_t<!std::is_function<T>::value &&
                       is_reference_wrapper<std::decay_t<RefWrap>>::value,
      decltype(ref.get().*pmd)>
  {
        return ref.get().*pmd;
  }

  template <typename Base, typename T, typename Pointer>
  auto INVOKE(T Base::*pmd, Pointer&& ptr)
      noexcept(noexcept((*std::forward<Pointer>(ptr)).*pmd))
   -> std::enable_if_t<!std::is_function<T>::value &&
                       !is_reference_wrapper<std::decay_t<Pointer>>::value &&
                       !std::is_base_of<Base, std::decay_t<Pointer>>::value,
      decltype((*std::forward<Pointer>(ptr)).*pmd)>
  {
        return (*std::forward<Pointer>(ptr)).*pmd;
  }

  template <typename F, typename... Args>
  auto INVOKE(F&& f, Args&&... args)
      noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
   -> std::enable_if_t<!std::is_member_pointer<std::decay_t<F>>::value,
      decltype(std::forward<F>(f)(std::forward<Args>(args)...))>
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

namespace detail {
  template <typename F, typename Tuple, std::size_t... I>
  constexpr decltype(auto) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> )
  {
    return cu::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
  }
} // namespace detail

template <typename F, typename Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
  return detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
      std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{});
}

} // namespace cu
