/** @file Defines macros for meaningful error reporting and the
 * class @c Exception.
 *
 * The macro @c CU_THROW_EXCEPTION throws such an exception with the
 * required information. It takes one argument which must be implicitely
 * convertable to std::string.
 *
 * The macro @c CU_ADD_EXCEPTION_CONTEXT can be used to provide additional
 * failure information to the caller in case of an exception. It adds one
 * exception to the chain of nested exceptions in case of an error.
 * The code
 *   @code
 *     try
 *     {
 *         doSomething();
 *     }
 *     catch(...)
 *     {
 *         CU_THROW_EXCEPTION( "Failed to do something." );
 *     }
 *   @endcode
 * is equivalent to
 *   @code
 *     CU_ADD_EXCEPTION_CONTEXT( "Failed to do something." )
 *     {
 *         doSomething();
 *     }; // note the semicolon.
 *   @endcode
 * This has several advantages:
 *   1. The name of the macro makes the intent clear.
 *   2. It avoids ugly try-catch control flow.
 * Note that the argument to the macro is evaluated lazily, even though
 * it does not look like it.
 *
 * The macro @c CU_ENFORCE tests a condition. If the condition is not
 * evaluated to @c true, then an exception with a specified error message is
 * thrown.
 *
 * The macro @c QU_ASSERT_THROW checks a condition to be true. If the
 * condition is not met, then @c assert() is used to check the condition
 * so in debug mode there is a way to debug the application right then and
 * there. Furthermore and exception is thrown with a custom message. This
 * also happens in release mode, even though the failed assertions indicates
 * a programming error.
 *
 * @author Ralph Tandetzky
 */

#pragma once

#include <cassert>
#include <exception>
#include <stdexcept>
#include <string>

namespace cu
{

struct ThrowSiteInfo
{
  explicit ThrowSiteInfo(
      const char * file_
    , int          line_
    , const char * date_
    , const char * time_ )
    : file(file_)
    , line(line_)
    , date(date_)
    , time(time_)
  {
  }

  const char * file = nullptr;
  int          line = 0;
  const char * date = nullptr;
  const char * time = nullptr;
};

// General macros for custom exception types.
#define CU_THROW_SITE_INFO ::cu::ThrowSiteInfo{ __FILE__, __LINE__, __DATE__, __TIME__ }
#define CU_THROW_SPECIFIC_EXCEPTION( msg, ExceptionType ) throw ExceptionType( msg, CU_THROW_SITE_INFO )
#define CU_ADD_EXCEPTION_CONTEXT_CUSTOM( Throw ) \
  ::cu::detail::makeExceptionContextAdder( [&](){Throw;} ) += [&]()

// Macros specifically for cu::Exceptions.
#define CU_THROW_EXCEPTION( msg ) CU_THROW_SPECIFIC_EXCEPTION( msg, ::cu::Exception )
#define CU_ADD_EXCEPTION_CONTEXT( msg ) \
  CU_ADD_EXCEPTION_CONTEXT_CUSTOM( CU_THROW_EXCEPTION(msg) )
#define CU_ENFORCE( expr, msg ) \
  if ( !static_cast<bool>(expr) ) { CU_THROW_EXCEPTION( msg ); }
#define CU_ASSERT_THROW( expr, msg ) \
  if ( !static_cast<bool>(expr) ) { assert(expr); CU_THROW_EXCEPTION( msg ); }


/// A general purpose exception class.
///
/// The @c Exception class stores an error message as well as information
/// about the source location and compilation time and possibly pending
/// exceptions. These can be used to produce detailed error messages.
///
/// The class also works well together with the C++ Standard Library,
/// since it derived from @c std::runtime_error and @c std::nested_exception,
/// so that unrelated libraries can (in theory) catch such an exception and
/// produce meaningful error messages.
///
/// Since the @c Exception class nests pending exceptions inside itself,
/// the user can get a detailed error message which is composed from the
/// error messages of the chain of nested exceptions.
class Exception
    : public std::runtime_error
    , public std::nested_exception
{
public:
  /// @c Exceptions should preferably be thrown by @c CU_THROW_EXCEPTION() or
  /// by some other macros within this file.
  Exception(const std::string & message, const ThrowSiteInfo & tsi_ )
    : std::runtime_error( message )
    , tsi( tsi_ )
  {}

  ThrowSiteInfo getThrowSiteInfo() const
  {
    return tsi;
  }

private:
  const ThrowSiteInfo tsi;
};


namespace detail
{

  template <typename Thrower>
  class ExceptionContextAdder
  {
  public:
    ExceptionContextAdder( Thrower && thrower_ )
      : thrower( std::forward<Thrower>(thrower_) )
    {
    }

    template <typename F>
    auto operator+=( F && f ) const &&
    {
      try
      {
        return std::forward<F>(f)();
      }
      catch(...)
      {
        std::forward<Thrower>(thrower)();
        // The following line is never reached, but avoids a warning.
        return std::forward<F>(f)();
      }
    }

  private:
    Thrower && thrower;
  };

  template <typename Thrower>
  ExceptionContextAdder<Thrower> makeExceptionContextAdder( Thrower && thrower )
  {
    return {std::forward<Thrower>(thrower)};
  }

} // namespace detail

} // namespace cu
