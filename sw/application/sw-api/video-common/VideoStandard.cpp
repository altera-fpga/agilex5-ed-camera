/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <iostream>
#include <sstream>
#include "VideoStandard.h"


std::string ColourSpaceToString(const TColourSpace& colorSpace)
{
    std::string colorSpaceString{"N/A"};

    if (colorSpace == TColourSpace::RGB)
        colorSpaceString = "RGB";
    else if (colorSpace == TColourSpace::Y444)
        colorSpaceString = "YUV 444";
    else if (colorSpace == TColourSpace::Y422)
        colorSpaceString = "YUV 422";
    else if (colorSpace == TColourSpace::Y420)
        colorSpaceString = "YUV 420";

    return colorSpaceString;        
}


uint32_t ColourDepthToNumber(const TColourDepth& colourDepth)
{
    uint32_t v = 0;

    if(colourDepth == TColourDepth::BPC8)
        v = 8;
    else if(colourDepth == TColourDepth::BPC10)
        v = 10;
    else if(colourDepth == TColourDepth::BPC12)
        v = 12;
    else if(colourDepth == TColourDepth::BPC16)
        v = 16;        

    return v;
}


bool TColourInfo::operator==(const TColourInfo& other) const
{
    return ((_space == other._space) and
            (_depth == other._depth) and
            (_colorimetry == other._colorimetry) and
            (_range == other._range));
}


// Default constructor initializes the video standard to a non-valid state
VideoStandard::VideoStandard():
    _width(0),
    _height(0),
    _height_f1(0),
    _frame_rate(0),
    _interlaced(false),
    _colour_info{TColourSpace::RGB, TColourDepth::BPC8, 0, 0}
{
}


void VideoStandard::SetWidth(const uint32_t v)
{
    _width = v;
}


void VideoStandard::SetHeight(const uint32_t v)
{
    _height = v;
}


void VideoStandard::SetFrameRate(const uint32_t v)
{
    _frame_rate = v;
}


void VideoStandard::SetInterlaced(const bool v)
{
    _interlaced = v;
}


void VideoStandard::SetColourSpace(const TColourSpace v)
{
    _colour_info._space = v;
}


void VideoStandard::SetColourDepth(const TColourDepth v)
{
    _colour_info._depth = v;
}


void VideoStandard::SetColourimetry(const uint32_t v)
{
    _colour_info._colorimetry = v;
}


void VideoStandard::SetColourRange(const uint32_t v)
{
    _colour_info._range = v;
}


bool VideoStandard::operator==(const VideoStandard& other) const
{
    return ((_width == other._width) and
            (_height == other._height) and
            (_height_f1 == other._height_f1) and
            (_frame_rate == other._frame_rate) and
            (_interlaced == other._interlaced) and
            (_colour_info == other._colour_info));
}


bool VideoStandard::IsValid() const
{
    if ((_width * _height) == 0)
        return false;
    else
        return (((_width % 2) == 0) and ((_height % 2) == 0));
}


std::string VideoStandard::ToString() const
{
    std::stringstream str;
    str << _width << "x" << _height;
    str << (_interlaced ? 'i' : 'p');

    if(_frame_rate)
    {
        str << " @" << _frame_rate / 100;
        
        if(uint32_t rm = _frame_rate % 100)
            str << "." << rm;
    }

    str << " " << ColourSpaceToString(_colour_info._space);
    str << " " << std::to_string(ColourDepthToNumber(_colour_info._depth));
    str << " bit";

    return str.str();
}


std::ostream& operator<<(std::ostream& os, const VideoStandard& vs)
{
    return os << (vs.ToString());
}