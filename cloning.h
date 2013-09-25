/// @file Never implement cloning manually anymore by
/// using this header file.
///
/// @author Ralph Tandetzky
/// @date 25 Sep 2013

#pragma once

#include <cassert>
#include <memory>

namespace cu {

/// Interface base class for all clonable objects.
class Clonable
{
public:
    virtual ~Clonable() {}

    std::unique_ptr<Clonable> clone() const
    {
        std::unique_ptr<Clonable> p( doClone() );
        assert( typeid(*p) == typeid(*this) );
        return p;
    }

private:
    virtual Clonable * doClone() const = 0;
};


template <typename T, typename Base = Clonable>
class AbstractClonable
        : public Base
{
public:
    std::unique_ptr<T> clone() const
    {
        std::unique_ptr<T> p( static_cast<T*>(doClone()) );
        assert( typeid(*p) == typeid(*this) );
        return p;
    }

private:
    virtual AbstractClonable * doClone() const override = 0;
};


template <typename T, typename Base = Clonable>
class ConcreteClonable
        : public AbstractClonable<T,Base>
{
private:
    virtual ConcreteClonable * doClone() const override
    {
        return new T(static_cast<T*>(this));
    }
};

} // namespace cu
