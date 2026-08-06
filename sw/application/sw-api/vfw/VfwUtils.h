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
using unpack_function_t = std::function<void(const uint8_t* __restrict, const std::size_t, uint8_t* __restrict)>;


// Extract packed 10 bit RGB samplses and store as 16 bit values
// Process data in 480 bit chunks (15 dwords, 16 pixels)
template<bool RGB = true>
void unpack_line_10_16(const uint8_t* __restrict src, const std::size_t num_bytes, uint8_t* __restrict dst_)
{
    const uint8_t* p1 = src;
    const uint8_t* const p2 = p1 + num_bytes;
    uint16_t* dst = reinterpret_cast<uint16_t*>(dst_);

    auto store_values = [&dst](const uint16_t r, const uint16_t g, const uint16_t b){
        if constexpr (RGB){        
            *dst++ = (b << 6);
            *dst++ = (g << 6);
        }
        *dst++ = (r << 6);
    };

    while(p1 < p2)
    {
        uint32_t* p = (uint32_t*)(p1);
        std::span<uint32_t> s(p, 15);

        uint32_t r = bitfield<at{ 9, 0}>::extract(s);
        uint32_t g = bitfield<at{19,10}>::extract(s);
        uint32_t b = bitfield<at{29,20}>::extract(s);
        store_values(r, g, b);
        
        r = bitfield<at{39,30}>::extract(s);
        g = bitfield<at{49,40}>::extract(s);
        b = bitfield<at{59,50}>::extract(s);
        store_values(r, g, b);


        r = bitfield<at{69,60}>::extract(s);
        g = bitfield<at{79,70}>::extract(s);
        b = bitfield<at{89,80}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{ 99, 90}>::extract(s);
        g = bitfield<at{109,100}>::extract(s);
        b = bitfield<at{119,110}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{129,120}>::extract(s);
        g = bitfield<at{139,130}>::extract(s);
        b = bitfield<at{149,140}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{159,150}>::extract(s);
        g = bitfield<at{169,160}>::extract(s);
        b = bitfield<at{179,170}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{189,180}>::extract(s);
        g = bitfield<at{199,190}>::extract(s);
        b = bitfield<at{209,200}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{219,210}>::extract(s);
        g = bitfield<at{229,220}>::extract(s);
        b = bitfield<at{239,230}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{249,240}>::extract(s);
        g = bitfield<at{259,250}>::extract(s);
        b = bitfield<at{269,260}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{279,270}>::extract(s);
        g = bitfield<at{289,280}>::extract(s);
        b = bitfield<at{299,290}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{309,300}>::extract(s);
        g = bitfield<at{319,310}>::extract(s);
        b = bitfield<at{329,320}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{339,330}>::extract(s);
        g = bitfield<at{349,340}>::extract(s);
        b = bitfield<at{359,350}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{369,360}>::extract(s);
        g = bitfield<at{379,370}>::extract(s);
        b = bitfield<at{389,380}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{399,390}>::extract(s);
        g = bitfield<at{409,400}>::extract(s);
        b = bitfield<at{419,410}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{429,420}>::extract(s);
        g = bitfield<at{439,430}>::extract(s);
        b = bitfield<at{449,440}>::extract(s);
        store_values(r, g, b);

        r = bitfield<at{459,450}>::extract(s);
        g = bitfield<at{469,460}>::extract(s);
        b = bitfield<at{479,470}>::extract(s);
        store_values(r, g, b);

        p1 += 60;
    }
}


// Extract packed SRC_BPS bit RGB samplses and store as DST_BPS bit values (16 bit max)
// Process input stream byte at a time
template<uint32_t SRC_BPS, uint32_t DST_BPS = 16, bool RGB = true>
inline void unpack_line_generic(const uint8_t* __restrict src, const std::size_t num_bytes, uint8_t* __restrict dst_)
{
    static constexpr uint32_t SRC_BITMASK = (1 << SRC_BPS) - 1;
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
            uint32_t value = (bitStore & SRC_BITMASK);

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

            channelCache[writeCount % 3] = value;
            writeCount++;

            if (((writeCount % 3) == 0) && writeCount)
            {
                if constexpr (RGB)
                {
                    *dst++ = (channelCache[2]);
                    *dst++ = (channelCache[1]);
                    *dst++ = (channelCache[0]);                                
                }
                else
                {
                    *dst++ = (channelCache[0]);
                }
            }
        }
    }
}


// Extract packed 12 bit RGB samplses and store as 16 bit values
// Process data in 288 bit chunks (9 dwords, 8 pixels)
template<bool RGB = true>
void unpack_line_12_16(const uint8_t* __restrict src, const std::size_t src_stride_bytes, uint8_t* __restrict dst_)
{
    const uint8_t* p1 = src;
    const uint8_t* const p2 = p1 + src_stride_bytes;
    uint16_t* dst = reinterpret_cast<uint16_t*>(dst_);

    auto store_values = [&dst](const uint16_t r, const uint16_t g, const uint16_t b){
        if constexpr (RGB){        
            *dst++ = (b << 4);
            *dst++ = (g << 4);
        }
        *dst++ = (r << 4);        
    };

    while(p1 < p2)
    {
        const uint32_t* p = (uint32_t*)(p1);

        uint32_t v = p[0];

        uint32_t r = v & 0xfff;
        uint32_t g = (v >> 12) & 0xfff;
        uint32_t b = (v >> 24) & 0xff;

        v = p[1];
        b |= ((v & 0xf) << 8);

        store_values(r, g, b);

        r = (v >> 4) & 0xfff;
        g = (v >> 16) & 0xfff;
        b = (v >> 28) & 0xf;

        v = p[2];
        b |= (v & 0xff) << 4;

        store_values(r, g, b);

        r = (v >> 8) & 0xfff;
        g = (v >> 20) & 0xfff;

        v = p[3];

        b = v & 0xfff;

        store_values(r, g, b);

        r = (v >> 12) & 0xfff;
        g = (v >> 24) & 0xff;

        v = p[4];
        
        g |= ((v & 0xf) << 8);
        b = (v >> 4) & 0xfff;

        store_values(r, g, b);        
        
        r = (v >> 16) & 0xfff;
        g = (v >> 28) & 0xf;

        v = p[5];

        g |= ((v & 0xff) << 4);
        b = (v >> 8) & 0xfff;

        store_values(r, g, b);
        
        r = (v >> 20) & 0xfff;

        v = p[6];

        g = v & 0xfff;
        b = (v >> 12) & 0xfff;

        store_values(r, g, b);
        
        r = (v >> 24) & 0xff;

        v = p[7];

        r |= ((v & 0xf) << 8);
        g = (v >> 4) & 0xfff;
        b = (v >> 16) & 0xfff;

        store_values(r, g, b);        

        r = (v >> 28) & 0xf;

        v = p[8];

        r |= ((v & 0xff) << 4);
        g = (v >> 8) & 0xfff;
        b = (v >> 20) & 0xfff;

        store_values(r, g, b);

        p1 += 36;
    }
}


template<bool RGB = true>
void unpack_line_16_16(const uint8_t* __restrict src, const std::size_t src_stride_bytes, uint8_t* __restrict dst_)
{
    const uint8_t* p1 = src;
    const uint8_t* const p2 = p1 + src_stride_bytes;
    uint16_t* dst = reinterpret_cast<uint16_t*>(dst_);

    auto store_values = [&dst](const uint16_t r, const uint16_t g, const uint16_t b){
        if constexpr (RGB){        
            *dst++ = b;
            *dst++ = g;
        }
        *dst++ = r;        
    };

    static constexpr uint32_t MASK = 0xffff;
    static constexpr uint32_t SHIFT = 16;

    while(p1 < p2)
    {
        const uint32_t* p = (uint32_t*)(p1);

        uint32_t v = p[0];

        uint32_t r = v & MASK;
        uint32_t g = (v >> SHIFT) & MASK;

        v = p[1];

        uint32_t b = v & MASK;

        store_values(r, g, b);

        r = (v >> SHIFT) & MASK;

        v = p[2];

        g = v & MASK;
        b = (v >> SHIFT) & MASK;

        store_values(r, g, b);

        p1 += 12;
    }    
}
