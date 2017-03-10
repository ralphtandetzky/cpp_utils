#pragma once

#include <cassert>

namespace cu
{

class ProgressInterface
{
public:
  virtual ~ProgressInterface() = default;
  virtual bool wasCanceled() const = 0;
  virtual void setProgress( double value ) = 0;
};


class PartialProgress
    : public ProgressInterface
{
public:
  PartialProgress(
      ProgressInterface & wrapped_,
      double min_,
      double max_ )
    : wrapped( wrapped_ )
    , min( min_ )
    , max( max_ )
  {
    assert( min >= 0 );
    assert( max >= min );
    assert( 1 >= max );
  }

  bool wasCanceled() const override
  {
    return wrapped.wasCanceled();
  }

  void setProgress( double value ) override
  {
    wrapped.setProgress( min + value*(max-min) );
  }

private:
  ProgressInterface & wrapped;
  const double min;
  const double max;
};

} // namespace cu
