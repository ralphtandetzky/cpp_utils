#pragma once

#include "polynomials.hpp"
#include "units.hpp"

#include <experimental/optional>
#include <vector>

namespace cu
{

template <typename T, std::size_t N>
struct FilterParams
{
  Polynomial<T,N> numerator;
  Polynomial<T,N> denominator;
};


template <typename T, std::size_t N>
class Filter
{
public:
  Filter( const FilterParams<T,N> & params_ )
    : params( params_ )
  {}

  T operator()( T in )
  {
    // calculate new value.
    T out = params.numerator[N] * in;
    for ( std::size_t i = 1; i <= N; ++i )
    {
      out += params.numerator  [N-i] * input [i-1];
      out -= params.denominator[N-i] * output[i-1];
    }
    out /= params.denominator[N];

    // shift input/output queues.
    for ( std::size_t i = N-1; i > 0; --i )
    {
      input [i] = input [i-1];
      output[i] = output[i-1];
    }
    input [0] = in ;
    output[0] = out;

    return out;
  }

private:
  FilterParams<T,N> params;
  std::array<T,N> input{}, output{};
};


// convert from s-plane to z-plane
template <typename T>
FilterParams<T,2> fromAnalogToDigital( const FilterParams<T,2> & filter )
{
  using placeholders::X;

  // s = (z-1)/(z+1) = p/q.
  const auto p = T(1) * X - T(1);
  const auto q = T(1) * X + T(1);
  return
  {
    q * q * filter.numerator  [0] +
    q * p * filter.numerator  [1] +
    p * p * filter.numerator  [2],
    q * q * filter.denominator[0] +
    q * p * filter.denominator[1] +
    p * p * filter.denominator[2]
  };
}

// convert from s-plane to z-plane
template <typename T>
FilterParams<T,1> fromAnalogToDigital( const FilterParams<T,1> & filter )
{
  using placeholders::X;

  // s = (z-1)/(z+1) = p/q.
  const auto p = T(1) * X - T(1);
  const auto q = T(1) * X + T(1);
  return
  {
    q * filter.numerator  [0] +
    p * filter.numerator  [1],
    q * filter.denominator[0] +
    p * filter.denominator[1]
  };
}


template <typename T>
struct CascadedFilterParams
{
  std::vector<FilterParams<T,2>> biquadFilters;
  std::experimental::optional<FilterParams<T,1>> bilinearFilter;

  template <typename F>
  void iterate( F && f )
  {
    for ( auto & filter : biquadFilters )
      f( filter );
    if ( bilinearFilter )
      f( bilinearFilter.value() );
  }
};


template <typename T>
CascadedFilterParams<T> fromAnalogToDigital( CascadedFilterParams<T> filter )
{
  filter.iterate( []( auto & filter )
  {
    filter = fromAnalogToDigital( filter );
  } );
  return std::move( filter );
}


template <typename T>
class CascadedFilter
{
public:
  CascadedFilter( const CascadedFilterParams<T> & params )
    : biquadFilters( params.biquadFilters.begin(),
                     params.biquadFilters.end() )
  {
    if ( params.bilinearFilter )
      bilinearFilter = Filter<T,1>{params.bilinearFilter.value()};
  }

  T operator()( T in )
  {
    for ( auto & filter : biquadFilters )
      in = filter( in );
    if ( bilinearFilter )
      in = bilinearFilter.value()( in );
    return in;
  }

private:
  std::vector<Filter<T,2>> biquadFilters;
  std::experimental::optional<Filter<T,1>> bilinearFilter;
};


template <typename T>
CascadedFilter<T> toFilter( const CascadedFilterParams<T> & params )
{
  return params;
}


template <typename T>
CascadedFilterParams<T> makeAnalogButterworthFilterParams(
    T cutoff,
    std::size_t degree
    )
{
  using placeholders::X;

  const auto N = degree;
  std::vector<FilterParams<T,2>> biquadFilters;
  biquadFilters.reserve( N/2 );
  const auto scaledX = 1/cutoff * X;
  for ( auto n = N*0; n < N/2; ++n )
    biquadFilters.push_back(
      { { 1 }, { T(1) + 2*sin(cu::pi*(n+0.5)/N)*scaledX + scaledX*scaledX } } );

  std::experimental::optional<FilterParams<T,1>> bilinearFilter;
  if ( N%2 == 1 )
    bilinearFilter = FilterParams<T,1>{ {1}, { T(1)+scaledX } };

  return { std::move( biquadFilters ), std::move( bilinearFilter ) };
}


/// @param cutoff Should be a value that is strictly between 0 and 0.5.
///   It is to be interpreted as relative to the sampling frequency,
///   i.e. the cutoff frequency is @c cutoff*samplingFrequency.
template <typename T>
CascadedFilterParams<T> makeDigitalButterworthFilterParams(
    T cutoff,
    std::size_t degree
    )
{
  const auto analogCutoff = std::tan( cu::pi * cutoff );
  return fromAnalogToDigital(
        makeAnalogButterworthFilterParams( analogCutoff, degree ) );
}


/// See @c makeDigitalButterworthFilterParams() for the meaning of the
/// parameters.
template <typename T>
CascadedFilter<T> makeButterworthFilter(
    T cutoff,
    std::size_t degree
    )
{
  return toFilter( makeDigitalButterworthFilterParams( cutoff, degree ) );
}


} // namespace cu
