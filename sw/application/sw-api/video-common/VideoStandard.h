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
#include <string>


enum class TColourSpace
{
	RGB,
	Y444,
	Y422,
	Y420
};


enum class TColourDepth
{
    BPC8,
    BPC10,
    BPC12,
    BPC16
};


struct TColourInfo
{
    TColourSpace _space;
    //uint32_t _depth;       /* 0 == 8 bit, 1 == 10 bit */
    TColourDepth _depth;
    uint32_t _colorimetry; /* SDR color (bt.709/sRGB) or HDR gamut (bt2020) */
    uint32_t _range;
	bool operator==(const TColourInfo& other) const;
	bool operator!=(const TColourInfo& other) const { return !(operator==(other)); }
};


struct VideoStandard
{
public:
    VideoStandard();

    constexpr VideoStandard(const uint32_t width, const uint32_t height, const uint32_t frame_rate, const TColourInfo& colour_info):
        _width(width),
        _height(height),
        _height_f1(height),
        _frame_rate(frame_rate),
        _interlaced(false),
        _colour_info(colour_info)
    {}

    constexpr uint32_t Width() const { return _width; }
    constexpr uint32_t Height() const { return _height; }
    constexpr uint32_t FrameRate() const { return _frame_rate; }         /* Frame rate x100 e.g. 59.94Hz incoded as 5994 */
    constexpr bool Interlaced() const { return _interlaced; }

    constexpr TColourSpace ColourSpace() const { return _colour_info._space; }
    constexpr TColourDepth ColourDepth() const { return _colour_info._depth; }
    constexpr uint32_t Colourimetry() const { return _colour_info._colorimetry; }
    constexpr uint32_t ColourRange() const { return _colour_info._range; }

    void SetWidth(const uint32_t v);
    void SetHeight(const uint32_t v);
    void SetFrameRate(const uint32_t v);  /* Frame rate x100 e.g. 59.94Hz incoded as 5994 */
    void SetInterlaced(const bool v);

    void SetColourSpace(const TColourSpace v);
    void SetColourDepth(const TColourDepth v);
    void SetColourimetry(const uint32_t v);
    void SetColourRange(const uint32_t v);

	bool operator==(const VideoStandard& other) const;
	bool operator!=(const VideoStandard& other) const { return !(operator==(other)); }
	bool IsValid() const;
    std::string ToString() const;

public:
    uint32_t _width;
    uint32_t _height;
    uint32_t _height_f1;
    uint32_t _frame_rate; /* Frame rate x100 e.g. 59.94Hz encoded as 5994 */  
    bool _interlaced;
    TColourInfo _colour_info;    
};


using ImageConfig = VideoStandard;


std::string ColourSpaceToString(const TColourSpace& colourSpace);
uint32_t ColourDepthToNumber(const TColourDepth& colourDepth);

std::ostream& operator<<(std::ostream& os, const VideoStandard& vs);
