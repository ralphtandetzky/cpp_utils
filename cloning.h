/// @file @brief Never implement cloning manually anymore!
///
/// @author Ralph Tandetzky
/// @date 25 Sep 2013

#pragma once

#include "cpp_utils/exception.h"
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
        CU_ASSERT_THROW( typeid(*p) == typeid(*this),
                         "Could not clone. "
                         "Incorrect dynamic type." );
        return p;
    }

private:
    virtual Clonable * doClone() const = 0;
};


/// Implements cloning for abstract base classes
template <typename T, typename Base = Clonable>
class AbstractClonable
        : public Base
{
public:
    std::unique_ptr<T> clone() const
    {
        std::unique_ptr<T> p( static_cast<T*>(doClone()) );
        CU_ASSERT_THROW( typeid(*p) == typeid(*this),
                         "Could not clone. "
                         "Incorrect dynamic type." );
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
