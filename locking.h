/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

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

template <typename T>
class Monitor
{
private:
    T t;
    mutable std::mutex m;

public:
    template <typename ... Ts>
    Monitor( Ts&&...ts ) : t(std::forward<Ts>(ts)... ) {}

    template <typename F>
    auto operator()( F f ) -> decltype(f(t))
    {
        std::lock_guard<std::mutex> lock(m);
        return f(t);
    }

    template <typename F>
    auto operator()( F f ) const -> decltype(f(t))
    {
        std::lock_guard<std::mutex> lock(m);
        return f(t);
    }
};

} // namespace cu
