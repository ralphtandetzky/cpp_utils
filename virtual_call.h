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
class VirtualCallImpl;

template <typename Ret, typename ...Args, typename F>
class VirtualCallImpl<Ret(Args...),F> /*final*/
    : public VirtualCall<Ret(Args...)>
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
