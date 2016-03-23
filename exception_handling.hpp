#pragma once

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
std::vector<std::exception_ptr> getNestedExceptionPtrs(
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

} // namespace cu
