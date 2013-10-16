/// @file
///
/// @author Ralph Tandetzky
/// @date 09 Oct 2013

#pragma once

#include <cassert>
#include <exception>
#include <stdexcept>

// Macros for throwing exceptions

#define CU_THROW_SITE_INFO \
    ::cu::ThrowSiteInfo{ __FILE__, __LINE__, __DATE__, __TIME__ }

#define CU_THROW_SPECIFIC_EXCEPTION(type,...) \
    throw type{__VA_ARGS__,CU_THROW_SITE_INFO}

#define CU_THROW(msg) \
    CU_THROW_SPECIFIC_EXCEPTION(::cu::Exception,msg)

#define CU_ASSERT_THROW(cond,msg) \
    ::cu::exception_detail::assert_throw( \
        (cond) ? "" : (msg), CU_THROW_SITE_INFO )

#define CU_THROW_USER_CANCELLED(msg) \
    CU_THROW_SPECIFIC_EXCEPTION(::cu::UserCancelledException,msg)


namespace cu {

struct ThrowSiteInfo
{
    const char * file;
    int line;
    const char * date;
    const char * time;

    bool isValid() const
    {
        return file != nullptr &&
               date != nullptr &&
               time != nullptr;
    }
};

class Exception
        : public std::runtime_error
        , public std::nested_exception
{
    using Base = std::runtime_error;
public:
    template <typename String>
    Exception( String && s, const ThrowSiteInfo & tsi )
        : Base{ std::forward<String>(s) }
        , tsi( tsi )
    {
    }

    ThrowSiteInfo getThrowSiteInfo() const noexcept
    {
        return tsi;
    }

private:
    ThrowSiteInfo tsi;
};

template <typename Tag>
class GenericException
        : virtual public Exception
{
public:
//    using Exception::Exception;
    template <typename ... Ts>
    GenericException( Ts&&...ts ) : Exception(std::forward<Ts>(ts)... ) {}
};

using UserCancelledException = GenericException<struct UserCancelledTag>;
using InternalError          = GenericException<struct InternalErrorTag>;

namespace exception_detail
{
    template <typename String>
    void assert_throw( String && msg, const ThrowSiteInfo & tsi )
    {
        if ( msg == std::string() )
            return;
        assert(!"Assertion failed. Throwing exception.");
        throw InternalError( std::forward<String>(msg), tsi );
    }
}

} // namespace cu
