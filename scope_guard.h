#pragma once

#include <cassert>

#define SCOPE_EXIT    SCOPE_IMPL( ScopeGuardExitType::Any     )
#define SCOPE_FAIL    SCOPE_IMPL( ScopeGuardExitType::Fail    )
#define SCOPE_SUCCESS SCOPE_IMPL( ScopeGuardExitType::Success )
#define SCOPE_IMPL(exitType) \
    auto guard ## __LINE__ = ScopeGuardImpl<exitType>() += [&]()

enum class ScopeGuardExitType
{
    Any,
    Fail,
    Success
};

template <typename F, ScopeGuardExitType E>
class ScopeGuard
{
public:
    ScopeGuard( F && f ) : f( std::forward<F>(f) ) {}
    ~ScopeGuard()
    {
        switch (E)
        {
        case ScopeGuardExitType::Any:
            f();
            break;
        case ScopeGuardExitType::Fail:
            if ( std::uncaught_exception() )
                f();
            break;
        case ScopeGuardExitType::Success:
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
class ScopeGuardImpl
{
    template <typename F>
    ScopeGuard<F,E> operator+=( F && f )
    {
        return ScopeGuard<F,E>( std::forward<F>(f) );
    }
};
