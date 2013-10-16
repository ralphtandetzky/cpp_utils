/// @file
///
/// @author Ralph Tandetzky
/// @date 25 Sep 2013

#pragma once

#include "cpp_utils/cow_ptr.h"
#include "cpp_utils/visitor.h"

#include <string>

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


class UserParameter
        : virtual public UserParameterVisitor::VisitableInterface
{
public:
    std::string getShortName  () const { return getImpl().shortName  ; }
    std::string getFullName   () const { return getImpl().fullName   ; }
    std::string getDescription() const { return getImpl().description; }
    size_t getIndex() const { return index; }
    void setIndex( size_t val ) { index = val; }

protected:
    // contains the immutable data of the user parameter.
    struct Impl
    {
        Impl(
              std::string && shortName
            , std::string && fullName
            , std::string && description
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

inline std::unique_ptr<UserParameter> clone(
        const UserParameter & param
        )
{
    return clone<UserParameterVisitor,UserParameter>( param );
}

void assign(
        UserParameter & dest
        , const UserParameter & src
        );


class RealUserParameter final
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
            std::move(shortName),
            std::move(fullName),
            std::move(description),
            lowerBound, upperBound, stepSize, nDecimals, suffix ), index )
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
        Impl( std::string && shortName
            , std::string && fullName
            , std::string && description
            , double lowerBound
            , double upperBound
            , double stepSize
            , size_t nDecimals
            , std::string suffix
            )
            : UserParameter::Impl(
                  std::move(shortName),
                  std::move(fullName),
                  std::move(description) )
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


class IntUserParameter final
    : public UserParameter
    , virtual public UserParameterVisitor::Visitable<IntUserParameter>
{
public:
    IntUserParameter( int value
                      , std::string shortName
                      , std::string fullName
                      , std::string description
                      , int lowerBound
                      , int upperBound
                      , int stepSize
                      , size_t index = 0 )
        : UserParameter( cu::cow_ptr<UserParameter::Impl>::make<Impl>(
            std::move(shortName),
            std::move(fullName),
            std::move(description),
            lowerBound, upperBound, stepSize ), index )
        , value(value)
    {}

    int  getLowerBound () const { return getImpl().lowerBound; }
    int  getUpperBound () const { return getImpl().upperBound; }
    int  getStepSize   () const { return getImpl().stepSize  ; }
    int  getValue      () const { return value; }
    void setValue( int val ) { value = val; }

protected:
    struct Impl : UserParameter::Impl
    {
        Impl( std::string && shortName
            , std::string && fullName
            , std::string && description
            , int lowerBound
            , int upperBound
            , int stepSize
            )
            : UserParameter::Impl(
                  std::move(shortName),
                  std::move(fullName),
                  std::move(description) )
            , lowerBound(std::move(lowerBound))
            , upperBound(std::move(upperBound))
            , stepSize  (std::move(stepSize  ))
        {}

        int lowerBound;
        int upperBound;
        int stepSize  ;
    };

    virtual const Impl & getImpl() const override
    {
        return static_cast<const Impl&>(UserParameter::getImpl());
    }

private:
    int value;
};


class BoolUserParameter final
    : public UserParameter
    , virtual public UserParameterVisitor::Visitable<BoolUserParameter>
{
public:
    BoolUserParameter( bool value
                      , std::string shortName
                      , std::string fullName
                      , std::string description
                      , size_t index = 0 )
        : UserParameter( cu::cow_ptr<UserParameter::Impl>::make<UserParameter::Impl>(
            std::move(shortName),
            std::move(fullName),
            std::move(description) ), index )
        , value(value)
    {}

    bool   getValue      () const { return value; }
    void   setValue( bool val ) { value = val; }

private:
    bool value;
};

} // namespace cu
