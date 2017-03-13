// This project underlies the optiMEAS Source Code License which is
// to be found at www.optimeas.de/source_code_license.

/** @file Defines the scope guard macros @c CU_SCOPE_EXIT, @c CU_SCOPE_FAIL
 * and @c CU_SCOPE_SUCCESS.
 *
 * These macros ensure, that certain statements are executed at the end of
 * the scope. This can be used for clean-up functions.
 *
 * These avoid user-defined RAII types and try catch block code
 * cluttering and provide good ways to accomplish exception-safety
 * and especially the strong exception guarantee.
 *
 * With scope guards, the code
 *   @code
 *     allocate(p);
 *     try
 *     {
 *         doSomething(p);
 *     }
 *     catch(...)
 *     {
 *         deallocate(p);
 *         throw;
 *     }
 *     deallocate(p);
 *   @endcode
 * can be rewritten to
 *   @code
 *     allocate(p);
 *     CU_SCOPE_EXIT { deallocate(p); }; // note the semicolon!
 *     doSomething(p);
 *   @endcode
 * Behind the scenes @c SCOPE_EXIT creates a temporary variable that calls
 * @c deallocate(p) in its destructor. This technique has several advantages:
 *   1. It clarifies intent.
 *   2. Resource acquisition and release are in one place.
 *   3. It avoids a deeply nested control flow.
 * Other than CU_SCOPE_EXIT, there is also the macro @c CU_SCOPE_FAIL with
 * the same syntax, but the code block behind the macro is only executed,
 * if the scope is left with an exception in flight. For completeness also
 * @c CU_SCOPE_SUCCESS is provided which calls the codeblock on scope exit
 * only in the case that no exception is in flight.
 *
 * @author Ralph Tandetzky
 */

#pragma once

namespace cu
{

#define CU_DETAIL_SCOPE_GUARD_CONCAT(a,b) CU_DETAIL_SCOPE_GUARD_CONCAT2(a,b)
#define CU_DETAIL_SCOPE_GUARD_CONCAT2(a,b) a##b
#define CU_DETAIL_SCOPE_GUARD_IMPL(Tag) \
  const auto CU_DETAIL_SCOPE_GUARD_CONCAT(scopeGuardVarName, __COUNTER__) = \
  ::cu::detail::ScopeGuardImpl<::cu::detail::Tag>() += [&]
#define CU_SCOPE_EXIT     CU_DETAIL_SCOPE_GUARD_IMPL(OnExitTag   )
#define CU_SCOPE_FAIL     CU_DETAIL_SCOPE_GUARD_IMPL(OnFailTag   )
#define CU_SCOPE_SUCCESS  CU_DETAIL_SCOPE_GUARD_IMPL(OnSuccessTag)

namespace detail
{

  struct OnExitTag    {};
  struct OnFailTag    {};
  struct OnSuccessTag {};

  inline bool shallExecuteScopeGuard( OnExitTag    ) { return true                      ; }
  inline bool shallExecuteScopeGuard( OnFailTag    ) { return  std::uncaught_exception(); }
  inline bool shallExecuteScopeGuard( OnSuccessTag ) { return !std::uncaught_exception(); }

  template <typename Tag>
  class ScopeGuardImpl
  {
    template <typename F>
    struct Type
    {
      ~Type()
      {
        if ( shallExecuteScopeGuard( Tag() ) )
          f();
      }

      F f;
    };

  public:
    template <typename F>
    Type<F> operator+=( F && f )
    {
      return {std::forward<F>(f)};
    }
  };

} // namespace detail

} // namespace cu
