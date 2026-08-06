//  Copyright (C) Altera Corporation
//
//  This code and the related documents are Altera copyrighted
//  materials and your use of them is governed by the express license
//  under which they were provided to you ("License"). This code and the
//  related documents are provided as is, with no express or implied
//  warranties other than those that are expressly stated in the License.

#pragma once

#include <cstdint>
#include <cstdlib>


struct VfrImage
{
    using buffer_t = std::unique_ptr<uint8_t, void(*)(uint8_t*)>;

    uint32_t _width = 0;
    uint32_t _height = 0;
    uint32_t _channels = 0;
    uint32_t _bps = 0;
    uint32_t _strideBytes = 0;
    uint32_t _sizeBytes = 0;
    buffer_t _pixels{nullptr, [](uint8_t*){}};

    VfrImage() = default;
    virtual ~VfrImage(){}
    VfrImage(VfrImage&&) = default;
    VfrImage& operator=(VfrImage&&) = default;
    VfrImage(const VfrImage&) = delete;
    VfrImage& operator=(const VfrImage&) = delete;
    
    VfrImage(uint32_t w, uint32_t h, uint32_t channels, uint32_t bps):
        _width{w},
        _height{h},
        _channels{channels},
        _bps{bps},
        _strideBytes{_width * _channels * ((_bps <= 8u) ? 1u : 2u)},
        _sizeBytes{_strideBytes * h},
        _pixels{(uint8_t*)malloc(_sizeBytes), [](uint8_t* p){ free(p); }}
    {}

    void AssignExternal(uint32_t w, uint32_t h, uint32_t channels, uint32_t bps,
                        buffer_t pixels, uint32_t strideRounding = 0)
    {
        auto roundup_pwr2 = [](std::size_t v, std::size_t r){
            return (v + (r - 1)) & ~(r - 1);
        };

        _width = w;
        _height = h;
        _channels = channels;
        _bps = bps;
        const uint32_t bytesPerSample = (bps <= 8u) ? 1u : 2u;
        _strideBytes = w * channels * bytesPerSample;

        if(strideRounding)
            _strideBytes = roundup_pwr2(_strideBytes, strideRounding);

        _sizeBytes = h * _strideBytes;
        _pixels = std::move(pixels);
    }

    uint8_t* Data() noexcept { return _pixels.get(); }
    const uint8_t* Data() const noexcept { return _pixels.get(); }
    uint32_t SizeBytes() const noexcept { return _sizeBytes; }
};