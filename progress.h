/** @file
  @author Ralph Tandetzky
  @date 07 May 2014
*/

#pragma once

#include "progress_interface.h"

namespace cu {

class PartialProgress final : public ProgressInterface
{
public:
    PartialProgress( ProgressInterface & parent, double from, double to )
        : parent(parent), from(from), to(to)
    {
        assert( from >= 0 );
        assert( to >= from );
        assert( to <= 1 );
    }

    virtual void setProgress( double progress )
    {
        assert( progress >= 0 );
        assert( progress <= 1 );
        parent.setProgress( from + progress*(to-from) );
    }

    virtual bool shallAbort() const
    {
        return parent.shallAbort();
    }

private:
    ProgressInterface & parent;
    const double from;
    const double to;
};


class ProgressForwarder final : public ProgressInterface
{
public:
    PartialProgress( ProgressInterface * parent )
        : parent(parent)
    {}

    virtual void setProgress( double progress )
    {
        assert( progress >= 0 );
        assert( progress <= 1 );
        if ( parent )
            parent->setProgress( progress );
    }

    virtual bool shallAbort() const
    {
        return parent ? parent->shallAbort() : false;
    }

private:
    ProgressInterface * const parent;
};

} // namespace cu
