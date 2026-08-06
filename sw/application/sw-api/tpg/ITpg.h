/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpTpg.h"
#include "TpgUtils.h"

namespace SwApi {

/**
 * High-level interface definition for the VVP Test Pattern Generator IP.
 */
struct ITpg 
{
    static std::shared_ptr<ITpg> Create(Hapi::VvpTpgPtr spTpg,
                                        uint32_t initialOutputWidth, uint32_t initialOutputHeight);
    virtual ~ITpg() {};

    /// Enables/disables the core depending on the value of `enable`, returning
    /// whether the operation succeeded.
    virtual bool SetEnable(bool enable) = 0;
    virtual bool GetEnable() = 0;

    virtual uint8_t GetBitsPerSample() = 0;

    virtual uint8_t GetNumberOfPatternsAvailable() = 0;
    virtual std::vector<eIntelVvpTpgPatternType> GetAvailablePatternsList() = 0;
    virtual intel_vvp_tpg_pattern GetLastSelectedPattern() = 0;
    virtual bool SetSelectedPattern(const intel_vvp_tpg_pattern& pattern) = 0;

    virtual bool SetOutputWidth(uint32_t newWidth) = 0;
    virtual uint32_t GetOutputWidth() = 0;

    virtual bool SetOutputHeight(uint32_t newHeight) = 0;
    virtual uint32_t GetOutputHeight() = 0;

    virtual bool SetColorsForUniformPattern(uint16_t color1, uint16_t color2, uint16_t color3) = 0;
    virtual std::tuple<uint16_t, uint16_t, uint16_t> GetColorsForUniformPattern() = 0;

    virtual bool SetZonePlateOrigin(uint16_t x_coord, uint16_t y_coord) = 0;
    virtual std::pair<uint16_t, uint16_t> GetZonePlateOrigin(void) = 0;
    virtual bool SetZonePlateFactors(uint16_t scaling_factor, uint16_t fine_tune_factor) = 0;
    virtual std::pair<uint16_t, uint16_t> GetZonePlateFactors(void) = 0;
};

}
