#pragma once

#include <exception>
#include <vector>

namespace cu
{

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
