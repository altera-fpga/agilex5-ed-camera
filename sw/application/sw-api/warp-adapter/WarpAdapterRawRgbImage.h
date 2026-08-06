/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

namespace SwApi
{
    template <typename uint_type>
    class RgbPixelTemplate
    {
    public:
        RgbPixelTemplate(uint_type red = 0, uint_type green = 0, uint_type blue = 0)
        :	_red(red)
        ,	_green(green)
        ,	_blue(blue)
        {
        }

        uint_type _red;
        uint_type _green;
        uint_type _blue;
    };

    using RgbPixel = RgbPixelTemplate<uint8_t>;
    using RgbPixel16 = RgbPixelTemplate<uint16_t>;
    
    template <typename uint_type>
    class RawRgbImageTemplate
    {
    public:
        RawRgbImageTemplate(uint32_t width, uint32_t height, std::shared_ptr<std::vector<uint8_t>> spData = nullptr)
        :	_width(width)
        ,	_height(height)
        ,	_spData(spData)
        {
            if (!_spData)
                _spData = std::make_shared<std::vector<uint8_t>>(width * height * sizeof(RgbPixelTemplate<uint_type>));
        }

        RgbPixelTemplate<uint_type>* GetPixel(uint32_t x, uint32_t y)
        {
            if ((x < _width) && (y < _height))
                return (RgbPixelTemplate<uint_type>*)&_spData->at(((y * _width) + x) * sizeof(RgbPixelTemplate<uint_type>));
            else
                return nullptr;
        }

        uint32_t GetWidth()
        {
            return _width;
        }

        uint32_t GetHeight()
        {
            return _height;
        }
        
        std::shared_ptr<std::vector<uint8_t>> GetData()
        {
            return _spData;
        }

    private:
        uint32_t _width;
        uint32_t _height;
        std::shared_ptr<std::vector<uint8_t>> _spData;
    };

    using RawRgbImage = RawRgbImageTemplate<uint8_t>;
    using RawRgbImage16 = RawRgbImageTemplate<uint16_t>;

}  // namespace SwApi