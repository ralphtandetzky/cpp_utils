/// @file
///
/// @author Ralph Tandetzky
/// @date 30 Aug 2013

#pragma once

#include "cpp_utils/exception.h"
#include "cpp_utils/more_algorithms.h"

#include <cassert>
#include <random>

namespace cu {

template < typename Container
         , typename CostFunction
         , typename ShallTerminateFunctor
         , typename SendBestFitFunctor
         , typename RandomNumberGenerator
           >
Container differentialEvolution(
        Container swarm,
        const double crossOverProbability,
        const double differentialWeight,
        CostFunction costFunction,
        ShallTerminateFunctor shallTerminate,
        SendBestFitFunctor sendBestFit,
        RandomNumberGenerator & rng )
{
    CU_ASSERT_THROW( swarm.size() >= 4,
                     "Swarm size of differential evolution "
                     "algorithm is too small." );
    const auto N = swarm[0].size();
    for ( auto & x : swarm )
        CU_ASSERT_THROW( x.size() == N,
                         "Invalid input to differential "
                         "evolution algorithm." );
    typedef decltype(costFunction(swarm[0])) Cost;
    std::vector<Cost> costs;
    std::transform( begin(swarm), end(swarm),
        back_inserter(costs), costFunction );
    auto lowestCostIndex = std::min_element(
        begin(costs), end(costs) ) - begin(costs);
    sendBestFit( swarm[lowestCostIndex],
                 costs[lowestCostIndex] );

    typedef typename Container::size_type size_type;
    std::uniform_int_distribution<> disX(0, swarm.size()-1);
    std::uniform_int_distribution<size_type> disR(0, N-1);
    std::uniform_real_distribution<> uniform;

    for (;;)
    {
        for ( size_type x = 0; x < swarm.size(); ++x )
        {
            if ( shallTerminate(swarm) )
                return swarm; // loop exit
            size_type a = 0, b = 0, c = 0;
            do a = disX(rng); while ( a == x );
            do b = disX(rng); while ( b == x || b == a );
            do c = disX(rng); while ( c == x || c == a || c == b );
            const auto R = disR(rng);
            auto y = swarm[x];
            for ( size_type i = 0; i < y.size(); ++i )
            {
                const auto r = uniform(rng);
                if ( r < crossOverProbability || i == R )
                    y[i] = swarm[a][i] + differentialWeight *
                            ( swarm[b][i] - swarm[c][i] );
            }
            const auto cost = costFunction(y);
            if ( cost < costs[x] )
            {
                swarm[x] = y;
                costs[x] = cost;
                if ( cost < costs[lowestCostIndex] )
                {
                    lowestCostIndex = x;
                    sendBestFit( swarm[lowestCostIndex],
                                 costs[lowestCostIndex] );
                }
            }
        }
    }
}


template < typename Container
         , typename CostFunction
         , typename ShallTerminateFunctor
         , typename SendBestFitFunctor
           >
Container nelderMead(
        Container swarm,
        CostFunction costFunction,
        ShallTerminateFunctor shallTerminate,
        SendBestFitFunctor sendBestFit,
        const double alpha = 1,    // reflection factor
        const double gamma = 2,    // expansion factor
        const double rho = -0.5,   // contraction factor
        const double sigma = 0.5 ) // reduction factor
{
    CU_ASSERT_THROW( !swarm.empty(),
                     "The array of initial values for the "
                     "Nelder Mead algorithm is empty." );
    const auto n = swarm.size()-1;
    CU_ASSERT_THROW( n > 0,
                     "There must be at least two initial "
                     "values for the Nelder Mead algorithm." );
    for ( auto & x : swarm )
        CU_ASSERT_THROW( x.size() == n,
                         "The dimensions of the initial values "
                         "of the Nelder Mead algorithm do not "
                         "correspond to the swarm size." );
    using Cost = decltype(costFunction(swarm[0]));
    using Rn = typename std::remove_reference<decltype(swarm[0])>::type;

    const auto add = [n]( Rn lhs, const Rn & rhs )
    {
        for ( size_t i = 0; i < n; ++i )
            lhs[i] += rhs[i];
        return lhs;
    };
    const auto sub = [n]( Rn lhs, const Rn & rhs )
    {
        for ( size_t i = 0; i < n; ++i )
            lhs[i] -= rhs[i];
        return lhs;
    };
    const auto mul = [n]( double f, Rn x )
    {
        for ( size_t i = 0; i < n; ++i )
            x[i] *= f;
        return x;
    };

    std::multimap<Cost,Rn*> xs;
    for ( auto & x : swarm )
        xs.insert( std::make_pair( costFunction(x),&x ) );
    sendBestFit( *xs.begin()->second, xs.begin()->first );

    while ( !shallTerminate( swarm ) )
    {
        const auto itn = std::prev( std::end(xs) );
        const auto pxn = itn->second;
        const auto fn  = itn->first; // == costFunction(*pxn)
        const auto it0 = std::begin(xs);
        const auto px0 = it0->second;
        const auto f0  = it0->first; // == costFunction(*px0)
        const auto fn_1= std::prev( std::end(xs), 2 )->first;

        const auto xo = mul( 1./n,
            sub( moving_accumulate( std::begin(swarm)+1, std::end(swarm),
                    swarm[0], add ), *pxn ) );
        // reflection
        const auto xr = add( xo, mul( alpha, sub( xo, *pxn ) ) );
        const auto fr = costFunction(xr);
        if ( fr < fn_1 && fr >= f0 )
        {
            *pxn = std::move(xr);
            xs.insert( std::make_pair( fr, pxn ) );
            xs.erase( itn );
            continue;
        }
        else if ( fr < f0 )
        {
            // expansion
            const auto xe = add( xo, mul( gamma, sub( xo, *pxn ) ) );
            const auto fe = costFunction(xe);
            *pxn = std::move( fe < fr ? xe : xr );
            const auto fbest = std::min( fe,fr );
            xs.insert( std::make_pair( fbest, pxn ) );
            xs.erase( itn );
            sendBestFit( *pxn, fbest );
            continue;
        }
        // contraction
        const auto xc = add( xo, mul( rho, sub( xo, *pxn ) ) );
        const auto fc = costFunction( xc );
        if ( fc < fn )
        {
            *pxn = std::move( xc );
            xs.insert( std::make_pair( fc, pxn ) );
            xs.erase( itn );
            continue;
        }
        // reduction
        xs.clear();
        for ( auto & x : swarm )
        {
            if ( &x == px0 )
            {
                xs.insert( std::make_pair( f0, &x ) );
            }
            else
            {
                x = add( *px0, mul( sigma, sub( x, *px0 ) ) );
                xs.insert( std::make_pair( costFunction(x), &x ) );
            }
        }
    }

    return swarm;
}


template < typename Canditate
         , typename GetNeighborFunction
         , typename CostFunction
         , typename TemperatureFunction
         , typename ShallTerminateFunctor
         , typename SendBestFitFunctor
         , typename RandomNumberGenerator
           >
Canditate simulatedAnnealing(
        Canditate init,
        GetNeighborFunction getNeighbor,
        CostFunction costFunction,
        TemperatureFunction getTemperature,
        ShallTerminateFunctor shallTerminate,
        SendBestFitFunctor sendBestFit,
        RandomNumberGenerator & rng )
{
    auto initCost = costFunction(init);
    auto best = init;
    auto bestCost = costFunction(best);
    sendBestFit( best, bestCost );
    std::uniform_real_distribution<> uniform;
    while ( !shallTerminate() )
    {
        const auto neighbor = getNeighbor(init);
        const auto neighborCost = costFunction(neighbor);
        if ( neighborCost > initCost )
        {
            if ( exp( (initCost - neighborCost) / getTemperature() ) > uniform(rng) )
            {
                init     = neighbor;
                initCost = neighborCost;
            }
            continue;
        }
        init     = neighbor;
        initCost = neighborCost;
        if ( initCost < bestCost )
        {
            best     = init;
            bestCost = initCost;
            sendBestFit( best, bestCost );
        }
    }
    return init;
}

} // namespace cu
