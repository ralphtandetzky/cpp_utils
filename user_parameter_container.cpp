#include "user_parameter_container.h"
#include "user_parameter.h"

#include <cassert>

namespace cu {

std::unique_ptr<UserParameter>
    UserParameterContainerBase::getParameter(
        size_t index ) const
{
    return ::cu::clone(*params[index]);
}


void UserParameterContainerBase::setParameter(
        const UserParameter & param )
{
    const auto index = param.getIndex();
    assert( index < params.size() );
    assign( *params.at(index), param );
}

} // namespace cu
