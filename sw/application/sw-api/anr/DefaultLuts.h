/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <vector>
#include <cstdint>

namespace SwApi
{
namespace AnrLuts
{
    static constexpr std::size_t INTENSITY_LUT_SIZE = 1024;
    static constexpr uint32_t UNITY_VALUE = 0x223d0;

    std::vector<uint32_t> GetUnityIntensityLut()
    {
        return std::vector<uint32_t>(INTENSITY_LUT_SIZE, UNITY_VALUE);
    }
    
    std::vector<uint32_t> GetUnitySpatialLut(uint8_t depth)
    {
        return std::vector<uint32_t>(depth, UNITY_VALUE);
    }

    std::vector<uint32_t> GetBlurIntensityLut()
    {
        return std::vector<uint32_t>(INTENSITY_LUT_SIZE, 0);
    }

    std::vector<uint32_t> GetBlurSpatialLut(uint8_t depth)
    {
        return std::vector<uint32_t>(depth, 0);
    }
}   // namespace AnrLuts
}   // namespace SwApi
