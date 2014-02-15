/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <cassert>

#define SCOPE_EXIT    SCOPE_IMPL( ScopeGuardExitType::any     )
#define SCOPE_FAIL    SCOPE_IMPL( ScopeGuardExitType::fail    )
#define SCOPE_SUCCESS SCOPE_IMPL( ScopeGuardExitType::success )
#define SCOPE_IMPL(exitType) \
    auto guard ## __LINE__ = ScopeGuardImpl<exitType>() += [&]()

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
