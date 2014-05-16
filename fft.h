#pragma once

#include "exception.h"
#include "math_constants.h"

#include <complex>
#include <vector>

namespace cu
{

namespace detail
{
    template <typename T>
    void cooleyTukey( std::complex<T> * v, std::complex<T> * w, size_t size )
    {
        using C = std::complex<T>;
        assert( (size & (size-1)) == 0 ); // size must be a power of two.
        if ( size < 2 )
            return;
        if ( size < 8 )
        {
            if ( size == 4 )
            {
                const auto i = C(0,1);
                w[0] = v[0] + v[2];
                w[1] = v[0] - v[2];
                w[2] = v[1] + v[3];
                w[3] = v[1] - v[3];
                v[0] = w[0]+  w[2];
                v[1] = w[1]+i*w[3];
                v[2] = w[0]-  w[2];
                v[3] = w[1]-i*w[3];
                return;
            }
            else if ( size == 2 )
            {
                w[0] = v[0]+v[1];
                v[1] = v[0]-v[1];
                v[0] = w[0];
                return;
            }
        }
        else // size >= 8
        {
            size_t half_size = size/2;
            for ( size_t i = 0; i < half_size; ++i )
            {
                w[i          ] = v[2*i];
                w[i+half_size] = v[2*i+1];
            }
            cooleyTukey( w          , v, half_size );
            cooleyTukey( w+half_size, v, half_size );
            const auto f = std::polar( T(1), 2*T(pi)/size);
            C twiddle = 1;
            for ( size_t i = 0, half_size = size/2; i < half_size; ++i )
            {
                const auto x = twiddle * w[i+half_size];
                v[i]           = w[i] + x;
                v[i+half_size] = w[i] - x;
                twiddle *= f;
            }
        }
    }

    template <typename T>
    std::vector<std::complex<T> > dftInQuadraticTime( std::vector<std::complex<T> > v )
    {
        using C = std::complex<T>;
        const auto size = v.size();
        std::vector<C> result(size,0.);
        for ( size_t i = 0; i < size; ++i )
        {
            C sum{};
            for ( size_t j = 0; j < size; ++j )
            {
                sum += std::polar( T{1}, 2*T{pi}*i*j/size ) * v[j];
            }
            result[i] = sum / sqrt(v.size());
        }

        return result;
    }

} // namespace detail

template <typename T>
std::vector<std::complex<T> > fft( std::vector<std::complex<T> > v )
{
    using C = std::complex<T>;
    const auto size = v.size();
    if ( size == 0 )
        return std::move(v);
    if ( (size & (size-1)) == 0 ) // size is a power of 2
    {
        detail::cooleyTukey( v.data(),
            std::vector<C>(size).data(), size );
        const T f = 1/sqrt(size);
        for ( auto & c : v )
            c*=f;
        return std::move(v);
    }
    else
        return detail::dftInQuadraticTime( std::move(v) );
}

} // namespace cu
