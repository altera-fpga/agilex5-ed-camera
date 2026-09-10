/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#pragma once

#include <cstdint>

namespace SwApi
{
    enum class IMX676Bps : uint8_t
    {
        RAW10 = 0x0,
        RAW12 = 0x1
    };

    enum class IMX676MipiLanes : uint8_t
    {
        TwoLanes = 0x1,
        FourLanes = 0x3,
        EightLanes = 0x7,
        FourLaneTwoChannel = 0x6
    };

    enum class IMX676ClockSpeed : uint8_t
    {
        kHz74250 = 0x0,
        kHz37125 = 0x1,
        kHz72000 = 0x2,
        kHz27000 = 0x3,
        kHz24000 = 0x4
    };

    enum class IMX676DataRate : uint8_t
    {
        Mbps2376 = 0x0,
        Mbps2079 = 0x1,
        Mbps1782 = 0x2,
        Mbps1440 = 0x3,
        Mbps1188 = 0x4,
        Mbps891 = 0x5,
        Mbps720 = 0x6,
        Mbps594 = 0x7
    };

    struct img676_reg
    {
        uint16_t addr;
        uint8_t val;
    };

} // namespace SwApi
