/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <cassert>
#include "macros.h"

#define CU_SCOPE_EXIT    CU_SCOPE_IMPL( ::cu::detail::ScopeGuardExitType::any     )
#define CU_SCOPE_FAIL    CU_SCOPE_IMPL( ::cu::detail::ScopeGuardExitType::fail    )
#define CU_SCOPE_SUCCESS CU_SCOPE_IMPL( ::cu::detail::ScopeGuardExitType::success )
#define CU_SCOPE_IMPL(exitType) \
    auto CU_UNIQUE_IDENTIFIER = ::cu::detail::ScopeGuardImpl<exitType>() += [&]()

namespace cu {

namespace detail {

enum class ScopeGuardExitType
{
    any,
    fail,
    success
};

template <typename F, ScopeGuardExitType E>
class ScopeGuard
{
public:
    explicit ScopeGuard( F && f ) : f( std::forward<F>(f) ) {}
    ~ScopeGuard() noexcept(false)
    {
        switch (E)
        {
        case ScopeGuardExitType::any:
            f();
            break;
        case ScopeGuardExitType::fail:
            if ( std::uncaught_exception() )
                f();
            break;
        case ScopeGuardExitType::success:
            if ( !std::uncaught_exception() )
                f();
            break;
        default:
            assert( !"Invalid case" );
        }
    }

private:
    F f;
};

template <ScopeGuardExitType E>
struct ScopeGuardImpl
{
    template <typename F>
    ScopeGuard<F,E> operator+=( F && f )
    {
        return ScopeGuard<F,E>( std::forward<F>(f) );
    }
};

} // namespace detail
} // namespace cu
