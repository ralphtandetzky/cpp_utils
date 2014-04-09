/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <cassert>

#define CU_SCOPE_EXIT    CU_SCOPE_IMPL( ::cu::ScopeGuardExitType::any     )
#define CU_SCOPE_FAIL    CU_SCOPE_IMPL( ::cu::ScopeGuardExitType::fail    )
#define CU_SCOPE_SUCCESS CU_SCOPE_IMPL( ::cu::ScopeGuardExitType::success )
#define CU_SCOPE_IMPL(exitType) \
    auto CU_CONCATENATE_TOKENS( guard, __LINE__ ) = ::cu::ScopeGuardImpl<exitType>() += [&]()
#define CU_CONCATENATE_TOKENS( x, y ) CU_CONCATENATE_TOKENS2( x, y )
#define CU_CONCATENATE_TOKENS2( x, y ) x ## y


namespace cu {

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
    ScopeGuard( F && f ) : f( std::forward<F>(f) ) {}
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

} // namespace cu
