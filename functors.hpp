#pragma once

namespace cu
{

struct NoOpFunctor
{
  template <typename ...Ts>
  void operator()( Ts&&... ) const
  {
  }
};

}
