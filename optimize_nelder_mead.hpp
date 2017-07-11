#pragma once

#include "array_arith.hpp"

#include <algorithm>

template <typename F,
          typename T,
          std::size_t N>
void optimizeNelderMead(
        F && f,
        std::array<std::array<T,N>,N+1> swarm,
        std::size_t nSteps
        )
{
    using namespace array_arith; // enables operators +, -, *, / for std::array.

    std::array<T,N+1> ys{};
    std::array<std::size_t,N+1> indexOrder;
    for ( std::size_t i = 0; i <= N; ++i )
    {
        indexOrder[i] = i;
        ys[i] = f(swarm[i]);
    }
    const auto sort = [&]
    {
        std::sort( begin(indexOrder), end(indexOrder),
                   [&]( std::size_t lhs, std::size_t rhs )
                   {
                       return ys[lhs] < ys[rhs];
                   } );
    };
    sort();
    const auto replace_worst = [&](
            const auto & new_x,
            const auto & new_y )
    {
        const auto pp = std::partition_point(
                    begin(indexOrder),
                    end  (indexOrder)-1,
                    [&]( const auto index )
                    { return ys[index] < new_y; } );
        std::rotate( pp, end(indexOrder)-1, end(indexOrder) );
        swarm[*pp] = new_x;
        ys   [*pp] = new_y;
    };

    while ( nSteps-- > 0 )
    {
        const auto sum = std::accumulate(
                    begin(swarm)+1,
                    end(swarm),
                    swarm.front(),
                    []( const auto & lhs, const auto & rhs )
                    { using namespace array_arith; return lhs + rhs; } );
        const auto x_last = swarm[indexOrder[N]];
        const auto y_last = ys[indexOrder[N]];
        const auto y_n = ys[indexOrder[N-1]];
        const auto x_o = (sum - x_last) / N;
        // reflect
        const auto x_r = x_o + x_o - x_last;
        const auto y_r = f( x_r );
        const auto x_1 = swarm[indexOrder[0]];
        const auto y_1 = ys[indexOrder[0]];
        if ( y_1 <= y_r && y_r < y_n )
        {
            replace_worst( x_r, y_r );
            continue;
        }
        // expand
        if ( y_r < y_1 )
        {
            const auto x_e = x_o + T(2) * ( x_r - x_o );
            const auto y_e = f( x_e );
            if ( y_e < y_r )
                replace_worst( x_e, y_e );
            else
                replace_worst( x_r, y_r );
            continue;
        }
        // contract
        const auto x_c = x_o + ( x_last - x_o ) / T(2);
        const auto y_c = f( x_c );
        if ( y_c < y_last )
        {
            replace_worst( x_c, y_c );
            continue;
        }
        // shrink
        for ( auto & x : swarm )
            x = (x_1 + x) / T(2);
        sort();
    }
}
