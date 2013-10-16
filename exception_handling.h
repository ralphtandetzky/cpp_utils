/// @file
///
/// @author Ralph Tandetzky
/// @date 09 Oct 2013

#pragma once

#include "cpp_utils/exception.h"

#include <algorithm>
#include <exception>
#include <vector>

namespace cu {


std::vector<std::exception_ptr> getExceptionChain( std::exception_ptr p )
{
    std::vector<std::exception_ptr> ptrs;
    while ( p != std::exception_ptr() )
    {
        ptrs.push_back( p );
        try
        {
            std::rethrow_exception( p );
        }
        catch ( const std::nested_exception & e )
        {
            p = e.nested_ptr();
        }
        catch ( ... )
        {
            break;
        }
    }
    return ptrs;
}

/// @pre @c p must not be a nullptr.
template <typename Ret, typename E, typename F1>
typename std::common_type<typename std::result_of<F1(E&)>::type,Ret>::type
    applyToException(
        std::exception_ptr p
        , F1 && f1
        , const Ret & otherwise )
{
    CU_ASSERT_THROW( p != std::exception_ptr(),
                     "An internal failure in error "
                     "handling procedures occurred." );
    try
    {
        std::rethrow_exception( p );
    }
    catch ( E & e )
    {
        return f1(e);
    }
    catch ( ... )
    {
        return otherwise;
    }
}

/// @pre @c p must not be a nullptr.
std::string getWhat( std::exception_ptr p )
{
    return applyToException<std::string,std::exception>(
                p,
                [](const std::exception & e)
                {return e.what();},
                "" );
}

/// @pre @c p must not be a nullptr.
ThrowSiteInfo getThrowSiteInfo( std::exception_ptr p )
{
    return applyToException<ThrowSiteInfo,Exception>(
                p,
                [](const Exception & e)
                {return e.getThrowSiteInfo();},
                ThrowSiteInfo() );
}

/// @pre @c p must not be a nullptr.
template <typename E>
bool isExceptionType( std::exception_ptr p )
{
    return applyToException<bool,E>(
                p,
                [](E&)
                {return true;},
                false );
}

/// @pre @c p must not be a nullptr.
bool isUserCancelledException( std::exception_ptr p )
{
    return isExceptionType<UserCancelledException>( p );
}

/// @pre @c p must not be a nullptr.
bool isInternalError( std::exception_ptr p )
{
    return isExceptionType<InternalError>( p );
}

std::vector<std::string> getWhatChain(
        const std::vector<std::exception_ptr> & ptrs )
{
    std::vector<std::string> whats;
    std::transform( begin(ptrs), end(ptrs),
                    back_inserter(whats), &getWhat );
    return whats;
}

std::vector<ThrowSiteInfo> getThrowSiteInfoChain(
        const std::vector<std::exception_ptr> & ptrs )
{
    std::vector<ThrowSiteInfo> tsis;
    std::transform( begin(ptrs), end(ptrs),
                    back_inserter(tsis), &getThrowSiteInfo );
    return tsis;
}

bool hasUserCancelledException(
        const std::vector<std::exception_ptr> & ptrs )
{
    return std::any_of( begin(ptrs), end(ptrs),
                        &isUserCancelledException );
}

} // namespace cu
