/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

 #pragma once

#include <cstdint>

namespace SwApi
{
    enum class IMX678Bps : uint8_t
    {
        RAW10 = 0x0,
        RAW12 = 0x1
    };

    enum class IMX678MipiLanes : uint8_t
    {
        TwoLanes = 0x1,
        FourLanes = 0x3,
        EightLanes = 0x6,
        FourLaneTwoChannel = 0x7
    };

    // When data rate is 525 MHz, must be either 36 or 72 clk
    enum class IMX678ClockSpeed : uint8_t
    {
        kHz74250 = 0x0,
        kHz37125 = 0x1,
        kHz72000 = 0x2,
        kHz27000 = 0x3,
        kHz24000 = 0x4,
        kHz36000 = 0x5,
        kHz18000 = 0x6,
        kHz13500 = 0x7
    };

    enum class IMX678DataRate : uint8_t
    {
        MHz2376 = 0x0,
        MHz2079 = 0x1,
        MHz1782 = 0x2,
        MHz1440 = 0x3,
        MHz1188 = 0x4,
        MHz891 = 0x5,
        MHz720 = 0x6,
        MHz594 = 0x7,
        MHz525 = 0x8,
        MHz1050 = 0x9
    };

    enum class IMX678OutputMode : uint8_t // Currently only supports AllPixel and Binning
    {
        AllPixel = 0,
        Binning = 1,
        DOL_AllPixel = 2,
        DOL_Binning = 3,
        ClearHDR_AllPixel = 4,
        ClearHDR_Binning = 5
    };

    struct IMX678ParamRequest
    {
        IMX678OutputMode _outputMode;
        // Clock stuff
        IMX678ClockSpeed _inputClock;
        IMX678MipiLanes _mipiLanes;

        // Output settings
        // uint16_t _outputWidth;
        // uint16_t _outputHeight;
        IMX678Bps _bps;
        uint16_t _targetFps;

        // Output from the setting system
        uint16_t _outputFps;

        // Cropping
        uint16_t _cropXStart;
        uint16_t _cropXWidth;
        uint16_t _cropYStart;
        uint16_t _cropYHeight;

        // Inversion
        bool _hInv;
        bool _vInv;
    };

    struct img678_reg
    {
        uint16_t addr;
        uint8_t val;        
    };

} // namespace SwApi