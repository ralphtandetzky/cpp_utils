#include "debug.h"
/*
#include <vector>
#include <boost/thread.hpp>
#include <iostream>

namespace cu {
namespace detail {
namespace { // unnamed

    struct Item
    {
        const char * scopeName;
        ThrowSiteInfo tsi;
    };

    boost::thread_specific_ptr<std::vector<Item>> items;

} // unnamed namespace

StackScopeTrace::StackScopeTrace(const char *scopeName, const ThrowSiteInfo &tsi)
{
    if ( items.get() == nullptr )
    {
        items.reset( new std::vector<Item> );
    }

    items.get()->push_back( {scopeName,tsi} );
}

StackScopeTrace::~StackScopeTrace()
{
    const auto p = items.get();
    assert( p );
    assert( !p->empty() );
    p->pop_back();
}

void showAssertMessage( const std::string & message )
{
    std::cerr << "Assertion failed.\n";
    std::cerr << message;
    const auto p = items.get();
    if ( p && !p->empty() )
    {
        std::cerr << "Scope trace:\n";
        auto i = 1;
        for ( const auto & x : *p )
        {
            std::cerr << i << ".";
            std::cerr << "\tName: " << x.scopeName;
            std::cerr << "\n\tSource File: " << x.tsi.file;
            std::cerr << ", Line: " << x.tsi.line;
            std::cerr << std::endl;
        }
    }

    assert( false );
}

} // namespace detail
} // namespace cu
*/
