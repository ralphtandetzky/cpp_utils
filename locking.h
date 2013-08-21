#pragma once

#include <mutex>

#define MAKE_LOCK_GUARD(m) std::lock_guard<std::mutex> lock ## __LINE__(m)
#define MAKE_UNIQUE_LOCK(m) auto lock ## __LINE__ = ::cu::MakeUniqueLock(m)

namespace cu {

template <typename M>
std::unique_lock<M> MakeUniqueLock( M & m )
{
    return std::unique_lock<M>( m );
}

} // namespace cu
