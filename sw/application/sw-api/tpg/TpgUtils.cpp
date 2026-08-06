/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "TpgUtils.h"

#include <iostream>

const char* ToString(const eIntelVvpTpgPatternType& type)
{
    switch (type)
    {
        case kIntelVvpTpgBarsPattern:
        {
            return "kIntelVvpTpgBarsPattern";
        }
        case kIntelVvpTpgUniformPattern:
        {
            return "kIntelVvpTpgUniformPattern";
        }
        case kIntelVvpTpgPathologicalPattern:
        {
            return "kIntelVvpTpgPathologicalPattern";
        }
        case kIntelVvpTpgZonePlatePattern:
        {
            return "kIntelVvpTpgZonePlatePattern";
        }
        case kIntelVvpTpgDigitalClockPattern:
        {
            return "kIntelVvpTpgDigitalClockPattern";
        }
        case kIntelVvpTpgSignalTapPattern:
        {
            return "kIntelVvpTpgSignalTapPattern";
        }
        case kIntelVvpTpgInvalidPattern:
        default:
        {
            return "kIntelVvpTpgInvalidPattern";
            break;
        }
    }
};

const char* ToString(const eIntelVvpTpgPatternColor& color)
{
    switch (color)
    {
        case kIntelVvpTpgRgb:
        {
            return "VvpTpgRgb";
        }
        case kIntelVvpTpgYcc444:
        {
            return "VvpTpgYcc444";
        }
        case kIntelVvpTpgYcc422:
        {
            return "VvpTpgYcc422";
        }
        case kIntelVvpTpgYcc420:
        {
            return "VvpTpgYcc420";
        }
        case kIntelVvpTpgMono:
        {
            return "VvpTpgMono";
        }
        case kIntelVvpTpgInvalidColor:
        default:
        {
            return "VvpTpgInvalidColor";
        }
    }

};

std::ostream& operator<<(std::ostream& o, const eIntelVvpTpgPatternType& type) {
    return o << ToString(type);
};
std::ostream& operator<<(std::ostream& o, const eIntelVvpTpgPatternColor& color) {
    return o << ToString(color);
};
std::ostream& operator<<(std::ostream& o, const intel_vvp_tpg_pattern& pattern) {
    return o << "intel_vvp_tpg_pattern(type: "  << pattern.type << ", " <<
                           "color: " << pattern.color << ")";
};
