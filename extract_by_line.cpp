#include <cpp_utils/extract_by_line.h>

#include "cpp_utils/exception.h"
#include "cpp_utils/scope_guard.h"

#include <istream>

namespace cu
{

std::vector<std::string> extractByLine( std::istream & is )
try
{
    const auto oldExceptions = is.exceptions();
    is.exceptions( std::istream::badbit );
    // restore exception flags upon exit.
    CU_SCOPE_EXIT {
        try
        {
            is.exceptions( oldExceptions );
        }
        catch (...)
        {
            if ( !std::uncaught_exception() )
                throw;
        }
    };

    std::vector<std::string> lines;
    std::string line;
    for (;;) // ever
    {
        assert( is.good() );
        std::getline( is, line ); // may throw
        lines.push_back( line );
        if ( is.eof() )
            return lines;
        else if ( is.fail() )
        {
            is.exceptions( std::istream::failbit ); // always throws.
            assert( false );
        }
    }
}
catch (...)
{
    CU_THROW( "Failed to extract lines from input stream." );
}


std::vector<std::string> extractByLine( std::istream && is )
{
    return extractByLine( is );
}

} // namespace cu
