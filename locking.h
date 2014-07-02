/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <mutex>

namespace cu {

/// Creates a std::unique_lock which has locked the mutex @c m. 
template <typename M>
std::unique_lock<M> MakeUniqueLock( M & m )
{
    return std::unique_lock<M>( m );
}

/// @c Helper class to implement the monitor pattern. 
///
/// This class wraps a type @c T and serializes all accesses to the 
/// contained data. If the type @c T is reentrant (in Qt terminology)
/// then Monitor<T> is thread-safe (unless the user of the class 
/// unlocks the held mutex willfully). 
///
/// The advantage of using this class instead of using a mutex directly
/// is that one cannot accidently forget to lock or unlock the mutex. 
/// Hence, hard-to-detect run-time errors are avoided. 
template <typename T>
class Monitor
{
private:
    T t;
    mutable std::mutex m;

public:
    /// Contructor. Forwards all arguments to the constructor of @c T.
    template <typename ... Ts>
    Monitor( Ts&&...ts ) : t(std::forward<Ts>(ts)... ) {}

    /// Locks the mutex and applies the functor to the wrapped data.
    template <typename F>
    auto operator()( F f ) -> decltype(f(t))
    {
        std::lock_guard<std::mutex> lock(m);
        return f(t);
    }

    /// Locks the mutex and applies the functor to the wrapped data.
    template <typename F>
    auto operator()( F f ) const -> decltype(f(t))
    {
        std::lock_guard<std::mutex> lock(m);
        return f(t);
    }

    /// Locks the mutex and applies the functor to the wrapped data
    /// and an @c std::unique_lock which locked the mutex. This function
    /// can be used to unlock the mutex temporarily, for example when
    /// waiting for a condition variable. 
    template <typename F>
    auto withUniqueLock( F f ) -> decltype(f(t,MakeUniqueLock(m)))
    {
        return f(t,MakeUniqueLock(m));
    }

    /// The const counter-part of withUniqueLock(). 
    template <typename F>
    auto withUniqueLock( F f ) const -> decltype(f(t,MakeUniqueLock(m)))
    {
        return f(t,MakeUniqueLock(m));
    }
};

} // namespace cu
