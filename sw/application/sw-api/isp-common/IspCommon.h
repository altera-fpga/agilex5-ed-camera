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

#include "AtUtils.h"
#include "VideoStandard.h"
#include "ICamera.h"

const char* ToString(const TCfaPhase& type);

std::ostream& operator<<(std::ostream& o, const TCfaPhase& patternType);

class RoiSelectorData
{
public:
    RoiSelectorData();
    std::string ToString(bool csv);
    static RoiSelectorData FromString(bool csvFormat, const std::string& strValue);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);

    bool _enabled;
    bool _outside;
    bool _tmo_enabled;
    float _x;
    float _y;
    float _width;
    float _height;
};


constexpr TColourInfo SimpleRgbColorInfo(){
    return {TColourSpace::RGB, TColourDepth::BPC8, 0, 0};
}


constexpr VideoStandard VS_2160p60_RGB8(){
    return VideoStandard{3840, 2160, 6000, SimpleRgbColorInfo()};
}


constexpr VideoStandard VS_1080p60_RGB8(){
    return VideoStandard{1920, 1080, 6000, SimpleRgbColorInfo()};
}
