/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "IDpc.h"
#include "DpcUtils.h"
#include "IspCommon.h"

#include <cstdint>

namespace SwApi {

namespace Dpc {

class DpcImplementation : public IDpc {
public:
    DpcImplementation(Hapi::VvpDpcPtr spDpc,
                      uint32_t initialOutputWidth, uint32_t initialOutputHeight);

    /// Enables/disables the core depending on the value of `bypass`, returning
    /// whether the operation succeeded.
    bool SetBypass(bool bypass) override;
    bool GetBypass() override;

    bool IsRunning() override;

    bool SetCfaPhase(TCfaPhase to) override;
    TCfaPhase GetCfaPhase() override;

    bool SetSensitivityLevel(DpcSensitivityLevel to) override;
    DpcSensitivityLevel GetSensitivityLevel() override;

    bool SetOutputWidth(uint32_t newWidth) override;
    uint32_t GetOutputWidth() override;

    bool SetOutputHeight(uint32_t newHeight) override;
    uint32_t GetOutputHeight() override;

    bool GetFrameStats(uint32_t *stats_out, uint32_t *pix_lo_count, uint32_t *pix_hi_count) override;
    bool StatsFreezeHandshake(void) override;
    bool SetStatsFreeze(bool state) override;

private:
    Hapi::VvpDpcPtr _spDpc = nullptr;

    uint32_t _outputWidth{0};
    uint32_t _outputHeight{0};
};

} // namespace Dpc

} // namespace SwApi
