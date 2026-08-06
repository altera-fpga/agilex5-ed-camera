/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#pragma once

#include "HapiVvpWbs.h"
#include <iosfwd>
#include <cstdint>

namespace SwApi {

const int wbs_region_zones_x = 7;
const int wbs_region_zones_y = 7;

struct WbsRoI
{
    uint16_t h_start;
    uint16_t v_start;
    uint16_t h_end;
    uint16_t v_end;
};

const char* ToString(const WbsRoI& roi);

struct NormalisedWbsRegion
{
    float red_strength;
    float green_strength;
    float blue_strength;
};

struct intel_vvp_wbs_zone_result_wide
{
    uint64_t x0_integer;
    uint32_t x0_fraction;
    uint64_t x1_integer;
    uint32_t x1_fraction;
    uint32_t num_pixels_accumulated;
};

const char* ToString(const NormalisedWbsRegion& normalisedRegion);

struct WbsResults
{
    intel_vvp_wbs_zone_result raw[wbs_region_zones_y][wbs_region_zones_x];
    NormalisedWbsRegion normalised[wbs_region_zones_y][wbs_region_zones_x];
};

// This enum controls the format of the output ratios
enum WbsResultFormat : uint8_t
{
    RB_GG = 0b00, // Red/Green, Blue/Green
    GB_RG = 0b01, // Green/Red, Blue/Green
    RG_GB = 0b10, // Red/Green, Green/Blue
    GG_RB = 0b11, // Green/Red, Green/Blue
    MAX_VAL = 0b100,
};

const char* ToString(const WbsResultFormat& format);

} // namespace SwApi

std::ostream& operator<<(std::ostream& o, const SwApi::WbsRoI& roi);
std::ostream& operator<<(std::ostream& o, const intel_vvp_wbs_zone_result& raw);
std::ostream& operator<<(std::ostream& o, const SwApi::NormalisedWbsRegion& normalisedRegion);
std::ostream& operator<<(std::ostream& o, const SwApi::WbsResultFormat& format);