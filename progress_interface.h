/** @file
  @author Ralph Tandetzky
  @date 07 May 2014
*/

#pragma once

namespace cu {

class ProgressInterface
{
public:
    virtual ~ProgressInterface() {}

    /// This function must be implementated thread-safely.
    /// @pre The value of @c progress must be between 0 and 1 inclusively.
    /// @pre Values passed to setProgress must never decrease.
    virtual void setProgress( double progress ) = 0;
    /// Returns if an operation shall be aborted usually due to a user
    /// request to cancel the operation. Operations should call this
    /// functions frequently, so they can abort the operation in a
    /// timely manner. This function must be implemented thread-safely.
    virtual bool shallAbort() const = 0;
};

} // namespace cu
