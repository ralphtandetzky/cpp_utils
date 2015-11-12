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
  inline bool shallExecuteScopeGuard( OnFailTag    ) { return !!std::current_exception(); }
  inline bool shallExecuteScopeGuard( OnSuccessTag ) { return ! std::current_exception(); }

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
