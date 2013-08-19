#pragma once

#include <mutex>

#define MAKE_LOCK_GUARD(m) std::lock_guard<std::mutex> lock ## __LINE__(m)
#define MAKE_UNIQUE_LOCK(m) auto lock ## __LINE__ = ::cu::make_unique_lock(m)

namespace cu {

template <typename M>
std::unique_lock<M> make_unique_lock( M & m )
{
    return std::unique_lock<M>( m );
}

} // namespace cu
