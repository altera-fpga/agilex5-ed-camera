//  Copyright (C) Altera Corporation
//
//  This code and the related documents are Altera copyrighted
//  materials and your use of them is governed by the express license
//  under which they were provided to you ("License"). This code and the
//  related documents are provided as is, with no express or implied
//  warranties other than those that are expressly stated in the License.

#pragma once

#include <functional>
#include <span>
#include "SwUtilsBitfield.h"

#include "arm_neon.h"

using namespace SwUtils;
using pack_function_t = std::function<void(const uint8_t* __restrict, const std::size_t, uint8_t* __restrict)>;


// Extract packed 10 bit mono/RGB samplses from store of 16 bit mono/RGB values
// For RGB out process data in 480 bit chunks (15 dwords, 16 pixels)
// For mono out process data in 160 bit chunks (5 dwords, 16 pixels)
template<bool out_RGB = true, bool in_RGB = true>
void pack_line_16_10(const uint8_t* __restrict src_, const std::size_t dst_stride_bytes, uint8_t* __restrict dst)
{
    const uint16_t* src = reinterpret_cast<const uint16_t*>(src_);
    uint8_t* p1 = dst;
    uint8_t* const p2 = p1 + dst_stride_bytes;

    if (out_RGB)
    {
        auto get_values = [&src]<typename T>(T& r, T& g, T& b){
            b = (T)((*src++) >> 6);
            if constexpr (in_RGB){
                g = (T)((*src++) >> 6);
                r = (T)((*src++) >> 6);
            }
            else
            {
                g = b;
                r = b;
            }
        };

        while(p1 < p2)
        {
            uint32_t r;
            uint32_t g;
            uint32_t b;
            uint32_t* p = (uint32_t*)(p1);
            std::span<uint32_t> s(p, 15);

            get_values(r, g, b);
            bitfield<at{ 9, 0}>::insert(s, r);
            bitfield<at{19,10}>::insert(s, g);
            bitfield<at{29,20}>::insert(s, b);
            
            get_values(r, g, b);
            bitfield<at{39,30}>::insert(s, r);
            bitfield<at{49,40}>::insert(s, g);
            bitfield<at{59,50}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{69,60}>::insert(s, r);
            bitfield<at{79,70}>::insert(s, g);
            bitfield<at{89,80}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{ 99, 90}>::insert(s, r);
            bitfield<at{109,100}>::insert(s, g);
            bitfield<at{119,110}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{129,120}>::insert(s, r);
            bitfield<at{139,130}>::insert(s, g);
            bitfield<at{149,140}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{159,150}>::insert(s, r);
            bitfield<at{169,160}>::insert(s, g);
            bitfield<at{179,170}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{189,180}>::insert(s, r);
            bitfield<at{199,190}>::insert(s, g);
            bitfield<at{209,200}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{219,210}>::insert(s, r);
            bitfield<at{229,220}>::insert(s, g);
            bitfield<at{239,230}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{249,240}>::insert(s, r);
            bitfield<at{259,250}>::insert(s, g);
            bitfield<at{269,260}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{279,270}>::insert(s, r);
            bitfield<at{289,280}>::insert(s, g);
            bitfield<at{299,290}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{309,300}>::insert(s, r);
            bitfield<at{319,310}>::insert(s, g);
            bitfield<at{329,320}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{339,330}>::insert(s, r);
            bitfield<at{349,340}>::insert(s, g);
            bitfield<at{359,350}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{369,360}>::insert(s, r);
            bitfield<at{379,370}>::insert(s, g);
            bitfield<at{389,380}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{399,390}>::insert(s, r);
            bitfield<at{409,400}>::insert(s, g);
            bitfield<at{419,410}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{429,420}>::insert(s, r);
            bitfield<at{439,430}>::insert(s, g);
            bitfield<at{449,440}>::insert(s, b);

            get_values(r, g, b);
            bitfield<at{459,450}>::insert(s, r);
            bitfield<at{469,460}>::insert(s, g);
            bitfield<at{479,470}>::insert(s, b);

            p1 += 60;
        }
    }
    else
    {
        auto get_value = [&src]<typename T>(T& a){
            a = (T)(*src++) >> 6;
            if constexpr (in_RGB){
                src++;
                src++;
            }
        };

        while(p1 < p2)
        {
            uint32_t a;
            uint32_t* p = (uint32_t*)(p1);
            std::span<uint32_t> s(p, 5);

            get_value(a);
            bitfield<at{ 9, 0}>::insert(s, a);
            get_value(a);
            bitfield<at{19,10}>::insert(s, a);
            get_value(a);
            bitfield<at{29,20}>::insert(s, a);
            get_value(a);
            bitfield<at{39,30}>::insert(s, a);
            get_value(a);
            bitfield<at{49,40}>::insert(s, a);
            get_value(a);
            bitfield<at{59,50}>::insert(s, a);
            get_value(a);
            bitfield<at{69,60}>::insert(s, a);
            get_value(a);
            bitfield<at{79,70}>::insert(s, a);
            get_value(a);
            bitfield<at{89,80}>::insert(s, a);
            get_value(a);
            bitfield<at{ 99, 90}>::insert(s, a);
            get_value(a);
            bitfield<at{109,100}>::insert(s, a);
            get_value(a);
            bitfield<at{119,110}>::insert(s, a);
            get_value(a);
            bitfield<at{129,120}>::insert(s, a);
            get_value(a);
            bitfield<at{139,130}>::insert(s, a);
            get_value(a);
            bitfield<at{149,140}>::insert(s, a);
            get_value(a);
            bitfield<at{159,150}>::insert(s, a);

            p1 += 20;
        }
    }
}


// Insert packed DST_BPS bit RGB samplses from store of SRC_BPS bit values (16 bit max)
// Process input stream byte at a time
template<uint32_t DST_BPS, uint32_t SRC_BPS = 16, bool out_RGB = true, bool in_RGB = true>
inline void pack_line_generic(const uint8_t* __restrict src, const std::size_t num_bytes, uint8_t* __restrict dst_)
{
    static constexpr uint32_t DST_BITMASK = (1 << DST_BPS) - 1;
    static constexpr int32_t DST_SHIFT = SRC_BPS - DST_BPS;

    const uint8_t* p = src;
    const uint8_t* const pend = p + num_bytes;
    uint16_t* dst = reinterpret_cast<uint16_t*>(dst_);

    uint32_t bitStore = 0;
    uint32_t bitCounter = 0;

    uint32_t writeCount = 0;
    uint16_t channelCache[3] = {0};

    while(p < pend)
    {
        bitStore |= ((*p++) << bitCounter);
        bitCounter += 8;

        if (bitCounter >= SRC_BPS)
        {
            uint32_t value = (bitStore & DST_BITMASK);

            if constexpr (DST_SHIFT >= 0)
            {
                value = value >> DST_SHIFT;
            }
            else
            {
                value = value << (-DST_SHIFT);
            }

            bitStore >>= SRC_BPS;
            bitCounter -= SRC_BPS;

            if (in_RGB)
            {
                channelCache[writeCount % 3] = value;
            }
            else
            {
                channelCache[0] = value;
                channelCache[1] = value;
                channelCache[2] = value;
            }
            writeCount++;

            if constexpr (out_RGB)
            {
                if (((writeCount % 3) == 0) && writeCount)
                {
                    *dst++ = (channelCache[2]);
                    *dst++ = (channelCache[1]);
                    *dst++ = (channelCache[0]);                                
                }
            }
            else
            {
                if (writeCount)
                {
                    *dst++ = (channelCache[0]);
                }
            }

        }
    }
}


// Extract packed 12 bit mono/RGB samplses from store of 16 bit mono/RGB values
// For RGB out process data in 288 bit chunks (9 dwords, 8 pixels)
// For mono out process data in 96 bit chunks (3 dwords, 8 pixels)
template<bool out_RGB = true, bool in_RGB = true>
void pack_line_16_12(const uint8_t* __restrict src_, const std::size_t dst_stride_bytes, uint8_t* __restrict dst)
{
    const uint16_t* src = reinterpret_cast<const uint16_t*>(src_);
    uint8_t* p1 = dst;
    uint8_t* const p2 = p1 + dst_stride_bytes;

    if (out_RGB)
    {
        auto get_values = [&src]<typename T>(T& r, T& g, T& b){
            if constexpr (in_RGB){
                b = (T)((*src++) >> 4);
                g = (T)((*src++) >> 4);
            }
            r = (T)((*src++) >> 4);
            if constexpr (!in_RGB){
                b = r;
                g = r;
            }
        };

        while(p1 < p2)
        {
            uint32_t r;
            uint32_t g;
            uint32_t b;
            uint32_t v;
            uint32_t* p = (uint32_t*)(p1);

            get_values(r, g, b);

            v  = r & 0xfff;
            v |= (g  & 0xfff) << 12;
            v |= (b  & 0xff) << 24;
            p[0] = v;

            v  = ((b & 0xf00) >> 8);
            
            get_values(r, g, b);

            v |= (r & 0xfff) << 4;
            v |= (g & 0xfff) << 16;
            v |= (b & 0xf) << 28;

            p[1] = v;

            v = (b & 0xff0) >> 4;

            get_values(r, g, b);

            v |= (r & 0xfff) << 8;
            v |= (b & 0xfff) << 20;

            p[2] = v;

            v = b & 0xfff;

            get_values(r, g, b);

            v |= (r & 0xfff) << 12;
            v |= (g & 0xff) << 24;

            p[3] = v;
            
            v  = ((g & 0xf00) >> 8);
            v |= (b & 0xfff) << 4;

            get_values(r, g, b);
            
            v |= (r & 0xfff) << 16;
            v |= (g & 0xf) << 28;

            p[4] = v;

            v  = ((g & 0xff0) >> 4);
            v |= (b & 0xfff) << 8;

            get_values(r, g, b);
            
            v |= (r & 0xfff) << 20;

            p[5] = v;

            v  = g & 0xfff;
            v |= (b & 0xfff) << 12;

            get_values(r, g, b);
            
            v |= (r & 0xff) << 24;

            p[6] = v;

            v  = ((r & 0xf00) >> 8);
            v |= (g & 0xfff) << 4;
            v |= (b & 0xfff) << 16;

            get_values(r, g, b);        

            v |= (r & 0xf) << 28;

            p[7] = v;

            v  = ((r & 0xff0) >> 4);
            g = (g & 0xfff) << 8;
            b = (b & 0xfff) << 20;

            p[8] = v;

            p1 += 36;
        }
    }
    else
    {
        auto get_value = [&src]<typename T>(T& a){
            a = (T)((*src++) >> 4);
            if constexpr (in_RGB)
            {
                src++;
                src++;
            }
        };

        while(p1 < p2)
        {
            uint32_t a;
            uint32_t v;
            uint32_t* p = (uint32_t*)(p1);

            get_value(a);
            v  = a & 0xfff;

            get_value(a);
            v |= (a  & 0xfff) << 12;

            get_value(a);
            v |= (a  & 0xff) << 24;

            p[0] = v;

            v  = ((a & 0xf00) >> 8);
           
            get_value(a);
            v |= (a & 0xfff) << 4;

            get_value(a);
            v |= (a & 0xfff) << 16;

            get_value(a);
            v |= (a & 0xf) << 28;

            p[1] = v;

            v = (a & 0xff0) >> 4;

            get_value(a);
            v |= (a & 0xfff) << 8;
            get_value(a);
            v |= (a & 0xfff) << 20;

            p[2] = v;
            p1 += 3;
        }
    }
}


// Extract packed 16 bit mono/RGB samplses from store of 16 bit mono/RGB values
// For RGB out process data in 96 bit chunks (3 dwords, 2 pixels)
// For mono out process data in 32 bit chunks (1 dwords, 2 pixels)
template<bool out_RGB = true, bool in_RGB = true>
void pack_line_16_16(const uint8_t* __restrict src_, const std::size_t dst_stride_bytes, uint8_t* __restrict dst)
{
    const uint8_t* p1 = dst;
    const uint8_t* const p2 = p1 + dst_stride_bytes;
    const uint16_t* src = reinterpret_cast<const uint16_t*>(src_);

    if (out_RGB)
    {
        auto get_values = [&src]<typename T>(T& r, T& g, T& b){
            b = (T)*src++;
            if constexpr (in_RGB)
            {
                g = (T)*src++;
                r = (T)*src++;
            }
            else
            {
                g = b;
                r = b;
            }
        };

        static constexpr uint32_t MASK = 0xffff;
        static constexpr uint32_t SHIFT = 16;

        while(p1 < p2)
        {
            uint32_t* p = (uint32_t*)(p1);

            uint32_t v;
            uint32_t r;
            uint32_t g;
            uint32_t b;
            
            get_values(r, g, b);

            v  = r & MASK;
            v |= ((g & MASK) << SHIFT);

            p[0] = v;

            v  = b & MASK;

            get_values(r, g, b);

            v |= ((r & MASK) << SHIFT);

            p[1] = v;

            v  = g & MASK;
            v |= ((b & MASK) << SHIFT);

            p[2] = v;

            p1 += 12;
        }    
    }
    else
    {
        auto get_value = [&src]<typename T>(T& a){
            a = (T)*src++;
            if constexpr (in_RGB)
            {
                src++;
                src++;
            }
        };

        static constexpr uint32_t MASK = 0xffff;
        static constexpr uint32_t SHIFT = 16;

        while(p1 < p2)
        {
            uint32_t* p = (uint32_t*)(p1);

            uint32_t v;
            uint32_t a;
            
            get_value(a);
            v  = a & MASK;
            get_value(a);
            v |= ((a & MASK) << SHIFT);

            p[0] = v;

            p1 += 4;
        }    
    }
}
