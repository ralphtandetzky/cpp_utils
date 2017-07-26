#pragma once

#include <cstdint>

namespace cu
{

template <typename T>
struct IntTraits;

namespace detail
{
    template <std::size_t nBits_,
              bool isSigned_>
    struct IntTraitsImpl
    {
        static constexpr std::size_t nBits = nBits_;
        static constexpr bool isSigned = isSigned_;
    };
}

template <> struct IntTraits<std::  int8_t> : detail::IntTraitsImpl< 8, true> {};
template <> struct IntTraits<std:: int16_t> : detail::IntTraitsImpl<16, true> {};
template <> struct IntTraits<std:: int32_t> : detail::IntTraitsImpl<32, true> {};
template <> struct IntTraits<std:: int64_t> : detail::IntTraitsImpl<64, true> {};
template <> struct IntTraits<std:: uint8_t> : detail::IntTraitsImpl< 8,false> {};
template <> struct IntTraits<std::uint16_t> : detail::IntTraitsImpl<16,false> {};
template <> struct IntTraits<std::uint32_t> : detail::IntTraitsImpl<32,false> {};
template <> struct IntTraits<std::uint64_t> : detail::IntTraitsImpl<64,false> {};

namespace detail
{
    template <std::size_t nBits,
              bool isSigned>
    struct BuildInIntImpl;

    template <typename T>
    struct TypeTrait { using type = T; };

    template <> struct BuildInIntImpl< 8, true> : TypeTrait<std::  int8_t> {};
    template <> struct BuildInIntImpl<16, true> : TypeTrait<std:: int16_t> {};
    template <> struct BuildInIntImpl<32, true> : TypeTrait<std:: int32_t> {};
    template <> struct BuildInIntImpl<64, true> : TypeTrait<std:: int64_t> {};
    template <> struct BuildInIntImpl< 8,false> : TypeTrait<std:: uint8_t> {};
    template <> struct BuildInIntImpl<16,false> : TypeTrait<std::uint16_t> {};
    template <> struct BuildInIntImpl<32,false> : TypeTrait<std::uint32_t> {};
    template <> struct BuildInIntImpl<64,false> : TypeTrait<std::uint64_t> {};
}

template <std::size_t nBits,
          bool isSigned = true>
using BuildInInt = typename detail::BuildInIntImpl<nBits,isSigned>::type;

template <typename IntT>
using DoubleSizeInt = BuildInInt<IntTraits<IntT>::nBits*2,IntTraits<IntT>::isSigned>;

} // namespace cu
