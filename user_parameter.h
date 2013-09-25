#pragma once

#include "cow_ptr.h"
#include "visitor.h"

namespace cu {

class RealUserParameter;
class  IntUserParameter;
class BoolUserParameter;
using UserParameterVisitor = Visitor<
     RealUserParameter
    , IntUserParameter
    ,BoolUserParameter
    >;
using ConstUserParameterVisitor = UserParameterVisitor::ConstVisitor;


class UserParameter : virtual public UserParameterVisitor::VisitableInterface
{
public:
    std::string getParameterShortName  () const { return getImpl().shortName  ; }
    std::string getParameterFullName   () const { return getImpl().fullName   ; }
    std::string getParameterDescription() const { return getImpl().description; }
    size_t getIndex() const { return index; }
    void setIndex( size_t val ) { index = val; }

protected:
    // contains the immutable data of the user parameter.
    struct Impl
    {
        Impl(
              std::string shortName
            , std::string fullName
            , std::string description
            )
            : shortName  (std::move(shortName  ))
            , fullName   (std::move(fullName   ))
            , description(std::move(description))
        {}

        std::string shortName  ;
        std::string fullName   ;
        std::string description;
    };

    virtual const Impl & getImpl() const { return *m; }

    UserParameter( cu::cow_ptr<Impl> m, size_t index = 0 )
        : m(m), index(index)
    {}

private:
    cu::cow_ptr<Impl> m;
    size_t index;
};


class RealUserParameter
    : public UserParameter
    , virtual public UserParameterVisitor::Visitable<RealUserParameter>
{
public:
    RealUserParameter( double value
                       , std::string shortName
                       , std::string fullName
                       , std::string description
                       , double lowerBound
                       , double upperBound
                       , double stepSize
                       , size_t nDecimals
                       , std::string suffix
                       , size_t index = 0 )
        : UserParameter( cu::cow_ptr<UserParameter::Impl>::make<Impl>(
            shortName, fullName, description, lowerBound,
            upperBound, stepSize, nDecimals, suffix ), index )
        , value(value)
    {}

    double getLowerBound () const { return getImpl().lowerBound; }
    double getUpperBound () const { return getImpl().upperBound; }
    double getStepSize   () const { return getImpl().stepSize  ; }
    size_t getNDecimals  () const { return getImpl().nDecimals ; }
    std::string getSuffix() const { return getImpl().suffix    ; }
    double getValue      () const { return value; }
    void   setValue( double val ) { value = val; }

protected:
    struct Impl : UserParameter::Impl
    {
        Impl( std::string shortName
            , std::string fullName
            , std::string description
            , double lowerBound
            , double upperBound
            , double stepSize
            , size_t nDecimals
            , std::string suffix
            )
            : UserParameter::Impl( shortName, fullName, description )
            , lowerBound(std::move(lowerBound))
            , upperBound(std::move(upperBound))
            , stepSize  (std::move(stepSize  ))
            , nDecimals (std::move(nDecimals ))
            , suffix    (std::move(suffix    ))
        {}

        double lowerBound;
        double upperBound;
        double stepSize  ;
        size_t nDecimals ;
        std::string suffix;
    };

    virtual const Impl & getImpl() const override
    {
        return static_cast<const Impl&>(UserParameter::getImpl());
    }

private:
    double value;
};


} // namespace cu
