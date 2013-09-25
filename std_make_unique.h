/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

#include <memory>

namespace std
{

template <typename T, typename...Args>
std::unique_ptr<T> make_unique( Args &&...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

} // namespace std
