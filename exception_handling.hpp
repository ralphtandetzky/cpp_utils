#pragma once

#include <cassert>
#include <exception>
#include <vector>

namespace cu
{

/// This function returns the chain of nested exceptions.
///
/// If an exception is thrown that inherits from @c std::nested_exception,
/// then the currently caught exception is stored in the
/// @c std::nested_exception subobject, if any. This caught exception
/// is said to be a nested exception. If this happens several times, then
/// there can be a whole chain of nested exceptions. This function returns
/// this chain of nested exceptions beginning with the passed exception
/// pointer, if it is not null.
inline std::vector<std::exception_ptr> getNestedExceptionPtrs(
    std::exception_ptr e_ptr = std::current_exception() )
{
  std::vector<std::exception_ptr> result{};

  while ( e_ptr )
  {
    result.push_back( e_ptr );
    try
    {
      std::rethrow_exception( e_ptr );
    }
    catch ( std::nested_exception & nested )
    {
      e_ptr = nested.nested_ptr();
    }
    catch ( ... )
    {
      break;
    }
  }

  return result;
}


namespace detail
{

  inline std::function<void()> & getGlobalExceptionHandlerRef()
  {
    static std::function<void()> globalExceptionHandler;
    return globalExceptionHandler;
  }

}


/// This function installs a global exception handler.
///
/// When calling the function @c handleException() the passed exception handler
/// will be called.
/// The passed exception handler must be thread-safe.
/// The @c handler object is called from within catch blocks only.
///
/// @note This function is not thread-safe, since it accesses static state
/// without mutex protection.
/// Also, this function should not be called while @c handleException() is
/// being called in another thread.
/// This should not be a problem, since this function should be called only
/// once at the start-up of an application before @c handleException() is
/// called.
///
/// In a gui application, the first thing that might be done in the @c main()
/// function is to call
///   @code
///     setGlobalExceptionHandler( getMessageBoxExceptionHandler() );
///   @endcode
/// In other applications exception messages might be written to a file.
inline void setGlobalExceptionHandler( std::function<void()> handler )
{
  detail::getGlobalExceptionHandlerRef().swap(handler);
}


/// To be used in a catch block to report errors to the user.
///
/// If this function is used outside a catch block, then the behavior is
/// undefined.
///
/// @note Before calling this function, a handling function must be installed
/// with @c setGlobalExceptionHandler().
/// The function @c setGlobalExceptionHandler() should never be called at the
/// same time as this function in an other thread, or undefined behaviour will
/// occur.
///
/// Otherwise, this function is thread-safe.
inline void handleException()
{
  assert( std::current_exception() );
  assert( detail::getGlobalExceptionHandlerRef() );
  detail::getGlobalExceptionHandlerRef()();
}


/// Use this macro to wrap a block of code from which all exceptions shall be
/// handled immediately.
///
/// Example:
///   @code
///     CU_HANDLE_ALL_EXCEPTIONS_FROM
///     {
///       possiblyThrowingOperation();
///     }; // <-- the semicolon is necessary!!
///   @endcode
/// This is equivalent to
///   @code
///     try
///     {
///       possiblyThrowingOperation();
///     }
///     catch(...)
///     {
///       handleException();
///     }
///   @endcode
/// If you try to return from the block (i.e., the keyword 'return' is used),
/// then the surrounding function will not return, but the
/// @c CU_HANDLE_ALL_EXCEPTIONS_FROM{...} expression will evaluate to the
/// return value. Therefore, the keyword 'return' needs to be prepended to
/// @c CU_HANDLE_ALL_EXCEPTIONS_FROM for this to work properly. If the return
/// type is not @c void and an exception is thrown, then a default constructed
/// object will be returned. For this reason, the return type must be void or
/// nothrow default constructible.
#define CU_HANDLE_ALL_EXCEPTIONS_FROM \
  ::cu::detail::HandleAllExceptionsFromImpl() += [&]()


namespace detail
{
  template <typename T>
  inline T getDefaultConstructed()
  {
    return {};
  }
  template <>
  inline void getDefaultConstructed<void>()
  {
  }

  struct HandleAllExceptionsFromImpl
  {
    template <typename F>
    auto operator+=( F && f ) const noexcept
    {
      using result_type = typename std::result_of<F()>::type;
      static_assert( std::is_same<result_type,void>::value ||
                     std::is_nothrow_default_constructible<result_type>::value,
                     "The return type of the block must be void or nothrow "
                     "default constructable. See the documentation of "
                     "CU_HANDLE_ALL_EXCEPTIONS_FROM." );
      try
      {
        return f();
      }
      catch(...)
      {
        handleException();
        return getDefaultConstructed<decltype(f())>();
      }
    }
};

} // namespace detail


/// Wraps a meta functor and handles all exceptions escaping the inside functor.
///
/// A meta functor is a class whose @c operator() takes a functor and executes
/// it, possibly passing a number of arguments. Examples are the @c cu::Monitor
/// class and the @c cu::TaskQueueThread classes. Taking such a class and
/// wrapping it with @c ExceptionHandlingInnerWrapper will manipulate the
/// functors before they are passed to the @c operator() in such a way that
/// all exceptions are caught and handled by @c handleException().
template <typename T>
class ExceptionHandlingInnerWrapper
{
private:
  T wrapped;

  template <typename F>
  class HandlingFunc
  {
  private:
    F f;

  public:
    HandlingFunc( F && f_ )
      : f( std::forward<F>(f_) )
    {
    }

    template <typename ...Args>
    auto operator()( Args &&... args )
    {
      return CU_HANDLE_ALL_EXCEPTIONS_FROM
      {
        return std::forward<F>(f)( std::forward<Args>(args)...);
      };
    }
  };

public:
  template <typename ...Args>
  ExceptionHandlingInnerWrapper( Args &&... args )
    : wrapped( std::forward<Args>(args)... )
  {
  }

  template <typename F>
  auto operator()( F && f ) noexcept
  {
    return wrapped( HandlingFunc<F>(std::forward<F>(f)) );
  }

};

} // namespace cu
