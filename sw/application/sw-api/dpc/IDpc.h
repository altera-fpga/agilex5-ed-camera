/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpDpc.h"
#include "DpcUtils.h"
#include "IspCommon.h"

#include <cstdint>

namespace SwApi {

enum class DpcCfaPhase : uint8_t;
enum class DpcSensitivityLevel : uint8_t;

/**
 * High-level interface definition for the VVP Defective Pixel Correction IP.
 *
 * The DPC IP attempts to detect and correct faulty pixels in a sensor's video
 * output. This is done by investigating the local region around each pixel,
 * and, according to a sensitivity level, correcting it to match the
 * neighbourhood.
 *
 * Use SetSensitivityLevel() and GetSensitivityLevel() for this.
 *
 * The DPC is a Bayer-only IP, so you MUST specify the correct CFA Phase using SetCfaPhase().
 */
struct IDpc {
    static constexpr auto VVP_DPC_PRODUCT_ID = INTEL_VVP_DPC_PRODUCT_ID;

    static std::shared_ptr<IDpc> Create(Hapi::VvpDpcPtr spDpc,
                                        uint32_t initialOutputWidth, uint32_t initialOutputHeight);
    virtual ~IDpc() {};

    /// Enables/disables the core depending on the value of `bypass`, returning
    /// whether the operation succeeded.
    virtual bool SetBypass(bool bypass) = 0;
    virtual bool GetBypass() = 0;

    virtual bool IsRunning() = 0;

    virtual bool SetCfaPhase(TCfaPhase to) = 0;
    virtual TCfaPhase GetCfaPhase() = 0;

    virtual bool SetSensitivityLevel(DpcSensitivityLevel to) = 0;
    virtual DpcSensitivityLevel GetSensitivityLevel() = 0;

    virtual bool SetOutputWidth(uint32_t newWidth) = 0;
    virtual uint32_t GetOutputWidth() = 0;

    virtual bool SetOutputHeight(uint32_t newHeight) = 0;
    virtual uint32_t GetOutputHeight() = 0;

    virtual bool GetFrameStats(uint32_t *stats_out, uint32_t *pix_lo_count, uint32_t *pix_hi_count) = 0;
    virtual bool StatsFreezeHandshake(void) = 0;
    virtual bool SetStatsFreeze(bool state) = 0;
};

}
