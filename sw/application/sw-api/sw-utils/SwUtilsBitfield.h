/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>

namespace SwUtils
{

template<uint32_t N>
constexpr uint32_t bitmask_single()
{
    return (1u << N);
}


template<uint32_t N>
constexpr uint32_t bitmask()
{
    uint32_t ret = bitmask_single<N>();

    if constexpr (N)
        ret |= bitmask<N-1>();

    return ret;
}


template<typename MSB, typename LSB>
struct at
{
    MSB _msb{};
    LSB _lsb{};

    constexpr auto lsb() const -> LSB {
        return _lsb;
    }

    constexpr auto msb() const -> MSB {
        return _msb;
    }    

    constexpr auto size() const -> size_t {
        return msb() - lsb() + 1u;
    }
};


// Template deduction guide
template<typename T, typename U>
at(T, U) -> at<T, U>;


template<auto At, std::unsigned_integral T = std::uint32_t>
struct bit_extractor
{
    template <typename R>
    constexpr static auto extract(R &&r)
    {
        using range_t = typename std::remove_cvref_t<R>::value_type;

        constexpr auto MSB = At.msb();
        constexpr auto LSB = At.lsb();

        constexpr size_t size_bits = sizeof(range_t) * 8;
        constexpr size_t idx_msb = MSB / size_bits;
        constexpr size_t idx_lsb = LSB / size_bits;
        constexpr size_t MSB_CURR = MSB % size_bits; 
        constexpr size_t LSB_CURR = std::max(static_cast<size_t>(LSB), idx_msb * size_bits) % size_bits;

        constexpr auto mask = bitmask<MSB_CURR>();
        constexpr auto shift = LSB_CURR;

        T rv = (std::forward<R>(r)[idx_msb] & mask) >> shift;

        if constexpr (idx_msb != idx_lsb)
        {
            constexpr size_t MSB_NEXT = idx_msb * size_bits - 1u;
            constexpr auto shift = MSB_NEXT - LSB + 1u;
            rv = (rv << shift) | bit_extractor<at{MSB_NEXT, LSB}>::extract(std::forward<R>(r));
        }

        return rv;
    }
};


template<auto At, std::unsigned_integral T = std::uint32_t>
struct bit_inserter
{
    template <typename R>
    constexpr static void insert(R &&r, T v)
    {
        using range_t = typename std::remove_cvref_t<R>::value_type;

        constexpr auto MSB = At.msb();
        constexpr auto LSB = At.lsb();

        constexpr size_t size_bits = sizeof(range_t) * 8;

        constexpr size_t idx_msb = MSB / size_bits;
        constexpr size_t idx_lsb = LSB / size_bits;

        constexpr size_t LSB_CURR = LSB % size_bits;
        constexpr size_t MSB_CURR = std::min(static_cast<size_t>(MSB), ((idx_lsb + 1) * size_bits) - 1) % size_bits;

        constexpr auto mask = bitmask<MSB_CURR-LSB_CURR>() << LSB_CURR;

        auto& e = std::forward<R>(r)[idx_lsb];

        e &= ~mask;
        e |= (v << LSB_CURR) & mask;

        if constexpr (idx_msb != idx_lsb)
        {
            constexpr size_t LSB_NEXT = (idx_lsb + 1) * size_bits;
            constexpr auto shift = MSB_CURR - LSB_CURR + 1u;
            bit_inserter<at{MSB, LSB_NEXT}>::insert(std::forward<R>(r), v >> shift);
        }
    }
};


template<auto... Ats>
struct bitfield
{
    template <typename R>
    constexpr static auto extract(R &&r)
    {
        auto rv = 0;

        (..., (rv <<= Ats.size(), rv += bit_extractor<Ats>::extract(r)));

        return rv;
    }

    template <typename R, std::unsigned_integral T = std::uint32_t>
    constexpr static void insert(R &&r, T v)
    {
        // If a field is split between multiple locations
        // the earlier location in the pack refers to
        // more significant bits of the value
        // We need to start from the least significat 
        // part of the value therefore need to iterate
        // the pack in the reverse order.
        // Full explanation how to do this:
        // https://www.foonathan.net/2020/05/fold-tricks/
        // Briefly this fold results in the expression similar to:
        // a = b = c = d
        // Where C++17 guarantees the d to be evaluated first
        // then c and so on.
        // (f(t),0) is a comma operator which calls f(t) first
        // then returns 0
        // So essentially we have the following assignment:
        // dummy = 0 = 0 = 0 with the insert() called along
        // for all the bitfield locations
        int dummy{};
        (dummy = ... = ((bit_inserter<Ats>::insert(r, v), v >>= Ats.size()), 0));
    }
};

} // namespace SwUtils