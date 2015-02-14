/// @file
///
/// @author Ralph Tandetzky
/// @date 7 Jan 2015

#pragma once
/*
#include "exception.h"
#include "macros.h"

#include <string>

#define CU_ADD_STACK_SCOPE( scopeName ) \
    auto CU_UNIQUE_IDENTIFIER = ::cu::detail::StackScopeTrace{ scopeName, CU_THROW_SITE_INFO }
#define CU_ADD_STACK_FUNC() \
    CU_ADD_STACK_SCOPE( __func__ )

#ifdef NDEBUG
#   define CU_ASSERT( cond, message )
#else // debug mode
#   define CU_ASSERT( cond, message ) \
        ::cu::detail::Assert( cond, message )
#endif // NDEBUG

namespace cu {

namespace detail {

class StackScopeTrace
{
public:
    StackScopeTrace( const char * scopeName, const ThrowSiteInfo & tsi );
    ~StackScopeTrace();
};

void showAssertMessage( const std::string & message );

template <typename String>
void Assert( bool cond, String && message )
{
    if ( !cond )
    {
        showAssertMessage( std::forward<String>(message) );
    }
}

} // namespace detail
} // namespace cu
*/
