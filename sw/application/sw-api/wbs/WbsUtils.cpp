
/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <iostream>
#include "WbsUtils.h"

namespace SwApi {

const char* ToString(const WbsResultFormat& format) {
    switch (format) {
        case WbsResultFormat::RB_GG: {
            return "RB_GG";
        }
        case WbsResultFormat::GB_RG: {
            return "GB_RG";
        }
        case WbsResultFormat::RG_GB: {
            return "RG_GB";
        }
        case WbsResultFormat::GG_RB: {
            return "GG_RB";
        }
        default: {
            return "WbsResultFormatInvalid";
        }
    }
}

} // namespace SwApi

std::ostream& operator<<(std::ostream& o, const intel_vvp_wbs_zone_result& raw) {
    return o << "x0_int: " << std::hex << raw.x0_integer << "\n"
             << "x0_frac: " << std::hex << raw.x0_fraction << "\n"
             << "x1_int: " << std::hex << raw.x1_integer << "\n"
             << "x1_frac: " << std::hex << raw.x1_fraction << "\n"
             << "num_pixels_accumulated: " << std::dec << raw.num_pixels_accumulated;
}

std::ostream& operator<<(std::ostream& o, const SwApi::NormalisedWbsRegion& region) {
    return o << "red: " << region.red_strength << ", " <<
                "green: " << region.green_strength << ", " <<
                "blue: " << region.blue_strength;
}

std::ostream& operator<<(std::ostream& o, const SwApi::WbsResultFormat& format) {
    return o << SwApi::ToString(format);
}
