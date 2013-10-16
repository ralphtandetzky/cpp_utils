/// @file
///
/// @author Ralph Tandetzky
/// @date 25 Sep 2013

#pragma once

#include "cpp_utils/cloning.h"
#include <string>
#include <vector>

// forward declarations
namespace cu { class UserParameter; }

namespace cu {

class UserParameterContainer
        : public AbstractClonable<UserParameterContainer>
{
public:
    virtual std::string getShortName() const = 0;
    virtual std::string getFullName() const = 0;
    virtual size_t getNParameters() const = 0;
    virtual std::unique_ptr<UserParameter> getParameter(
            size_t index ) const = 0;
    virtual void setParameter( const UserParameter & ) = 0;
};


class UserParameterContainerBase
        : public AbstractClonable<
            UserParameterContainerBase
            ,UserParameterContainer>
{
public:
    UserParameterContainerBase(
            std::string shortName
            , std::string fullName )
        : shortName(shortName)
        , fullName(fullName)
    {
    }

    UserParameterContainerBase(
            const UserParameterContainerBase & other ) = delete;

    virtual std::string getShortName() const override
    {
        return shortName;
    }

    virtual std::string getFullName() const override
    {
        return fullName;
    }

    virtual size_t getNParameters() const override
    {
        return params.size();
    }

    virtual std::unique_ptr<UserParameter> getParameter(
            size_t index ) const override;

    virtual void setParameter(
            const UserParameter & param ) override;

private:
    std::string shortName;
    std::string fullName;
    std::vector<UserParameter*> params;
};

} // namespace cu
