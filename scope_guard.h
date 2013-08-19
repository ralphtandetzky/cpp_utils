#pragma once

#include <cassert>

#define SCOPE_EXIT    SCOPE_IMPL( scope_guard_exit_t::any     )
#define SCOPE_FAIL    SCOPE_IMPL( scope_guard_exit_t::fail    )
#define SCOPE_SUCCESS SCOPE_IMPL( scope_guard_exit_t::success )
#define SCOPE_IMPL(exit_type) \
    auto guard ## __LINE__ = scope_guard_impl<exit_type>() += [&]()

namespace cu {

enum class scope_guard_exit_t
{
    any,
    fail,
    success
};

template <typename F, scope_guard_exit_t E>
class scope_guard
{
public:
    scope_guard( F && f ) : f( std::forward<F>(f) ) {}
    ~scope_guard()
    {
        switch (E)
        {
        case scope_guard_exit_t::any:
            f();
            break;
        case scope_guard_exit_t::fail:
            if ( std::uncaught_exception() )
                f();
            break;
        case scope_guard_exit_t::success:
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

template <scope_guard_exit_t E>
struct scope_guard_impl
{
    template <typename F>
    scope_guard<F,E> operator+=( F && f )
    {
        return scope_guard<F,E>( std::forward<F>(f) );
    }
};

} // namespace cu
