/// @file
///
/// @author Ralph Tandetzky
/// @date 14 Aug 2013

#pragma once

namespace cu {

template <typename ...>
class VirtualCall;


template <typename Ret, typename ...Args>
class VirtualCall<Ret(Args...)>
{
public:
    virtual Ret operator()( Args &&... args ) = 0;

protected:
    ~VirtualCall() {}
};


template <typename ...>
class VirtualCallAndConstCall;

template <typename Ret, typename ...Args>
class VirtualCallAndConstCall<Ret(Args...)>
        : public VirtualCall<Ret(Args...)>
{
public:
    virtual Ret operator()( Args &&... args ) const = 0;

protected:
    ~VirtualCallAndConstCall() {}
};



template <typename F, template ...Args>
struct Invokable {
    template <typename U = F>
    static auto test(int) ->
        decltype(std::declval<typename std::result_of<F(Args...)>::type>(), std::true_type());

    static auto test(...) -> false_type;

    static constexpr bool value = decltype(test(0))::value;
};


template <typename ...>
class VirtualCallImpl;

template <typename Ret, typename ...Args, typename F>
class VirtualCallImpl<Ret(Args...),F> final
    : public std::conditional<Invokable<F const(Args...)>::value,
        VirtualCallAndConstCall<Ret(Args...)>,
        VirtualCall<Ret(Args...)>::type
{
public:
    VirtualCallImpl( F && f )
        : f( std::forward<F>(f) )
    {
    }

    virtual Ret operator()( Args &&... args ) /*override*/
    {
        return f( std::forward<Args>(args)... );
    }

private:
    F f;
};


template <typename Ret, typename ...Args, typename F>
VirtualCallImpl<Ret(Args...),F> makeVirtual( F && f )
{
    return VirtualCallImpl<Ret(Args...),F>( std::forward<F>(f) );
}

} // namespace cu
