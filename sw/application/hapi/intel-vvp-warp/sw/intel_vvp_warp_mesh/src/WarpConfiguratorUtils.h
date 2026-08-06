/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#pragma once

#include <numbers>


namespace intel_vvp_warp
{

constexpr inline float degrees_to_radians(float a)
{
    return a * (std::numbers::pi_v<float>  / 180.0f);
}

constexpr inline float radians_to_degrees(float a)
{
    return a / (std::numbers::pi_v<float> / 180.0f);
}

constexpr inline float get_norm_val(float v, float f)
{
    return f > 0.0f ? v / f : 0.0f;
}

constexpr inline float get_denorm_val(float v, float f)
{
    return  roundf(v * f);
}

}