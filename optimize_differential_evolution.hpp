#pragma once

#include "array_arith.hpp"
#include "vector_arith.hpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace cu
{

template <typename F,
          typename Vector,
          typename RNG>
void optimizeDifferentialEvolution(
        F && f,
        std::vector<Vector> swarm,
        RNG & rng,
        std::size_t nSteps
        )
{
    const auto size = swarm.size();
    std::vector<decltype(f(swarm[0]))> values;
    values.reserve( size );
    for ( const auto & x : swarm )
        values.push_back( f(x) );

    std::uniform_int_distribution<std::size_t> dist(0,size-1);
    Vector y;
    while ( nSteps-- != 0 )
    {
        std::size_t i=0;
        std::size_t j=0;
        std::size_t k=0;
        std::size_t l=0;
        i = dist(rng);
        do { j = dist(rng); } while ( i == j );
        do { k = dist(rng); } while ( i == k || j == k );
        do { l = dist(rng); } while ( i == l || j == l || k == l );
        auto &       x = swarm[i];
        using namespace array_arith;
        using namespace vector_arith;
        assign( y, swarm[j] + swarm[k] - swarm[l] );
        auto &     f_x = values[i];
        const auto f_y = f(y);
        if ( f_y < f_x )
        {
            x.swap(y);
            f_x = f_y;
        }
    }
}

} // namespace cu
