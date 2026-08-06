/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "VvpCoreBase.h"
#include "WbsUtils.h"
#include "IspCommon.h"
#include <atomic>
#include <thread>

#include <cstdint>

namespace SwApi {

class Wbs : public VvpCoreBase
{
public:
    static std::shared_ptr<Wbs> Create(Hapi::VvpWbsPtr spWbs);
    Wbs(Hapi::VvpWbsPtr spWbs);

    bool SetBypass(bool bypass, UpdatePolicy policy = UpdatePolicy::Async());
    bool GetBypass();
    bool SetStatsFreeze(bool freeze);
    bool GetStatsFreezeSetting();

    bool IsRunning(void);

    bool SetResolution(uint32_t width, uint32_t height);
    std::pair<uint32_t, uint32_t> GetResolution();

    uint16_t GetPrecisionBits(void);

    bool SetCfaPhase(TCfaPhase to, UpdatePolicy policy = UpdatePolicy::Async());
    TCfaPhase GetCfaPhase(void);

    bool SetResultFormat(WbsResultFormat format, UpdatePolicy policy = UpdatePolicy::Async());
    WbsResultFormat GetResultFormat(void);

    WbsRoI GetClosestValidIdealRoI(WbsRoI& roi);
    bool SetRoI(WbsRoI& roi, UpdatePolicy policy = UpdatePolicy::Async());
    WbsRoI GetRoI(void);

    bool SetCfaX0Ranges(uint32_t upper, uint32_t lower, UpdatePolicy policy = UpdatePolicy::Async());
    bool SetCfaX0Ranges(float upper, float lower, UpdatePolicy policy = UpdatePolicy::Async());
    bool GetCfaX0Ranges(uint32_t &upper, uint32_t &lower);
    bool GetCfaX0Ranges(float &upper, float &lower);

    bool SetCfaX1Ranges(uint32_t upper, uint32_t lower, UpdatePolicy policy = UpdatePolicy::Async());
    bool SetCfaX1Ranges(float upper, float lower, UpdatePolicy policy = UpdatePolicy::Async());
    bool GetCfaX1Ranges(uint32_t &upper, uint32_t &lower);
    bool GetCfaX1Ranges(float &upper, float &lower);

    bool StatsFreezeHandshake(void);
    WbsResults ReadResultTable(void);

    NormalisedWbsRegion ConvertRawToNormalisedResult(intel_vvp_wbs_zone_result raw);
    NormalisedWbsRegion ConvertWideRawToNormalisedResult(intel_vvp_wbs_zone_result_wide raw);

    WbsResultFormat GetOptimalResultFormatForCurrentSetup();

private:
    void WaitForPendingRead();
    bool CommitSettings() override;
    bool IsCommitPending() override;

    Hapi::VvpWbsPtr _spWbs = nullptr;

    uint32_t _width;
    uint32_t _height;

    WbsRoI _wbsRoI;
    bool _validRoI = false;

    TCfaPhase _cfaPhase = TCfaPhase::RGGB;
    WbsResultFormat _resultFormat = WbsResultFormat::RG_GB;

    std::atomic<bool> _read_pending = false;
    WbsResults _results = {0};

    // Default config for RGGB + RB_GG format
    bool _invertRed = false;
    bool _invertBlue = true;
    bool _swapRedBlue = false;

    // InvertLUT: if cfa & 0x2, these are inverted
    // Format is {_invertBlue, _invertRed}
    bool _invertLUT[WbsResultFormat::MAX_VAL][2] =
    {
        {false,  true}, // RB_GG
        {true,  true}, // GB_RG
        {false, false}, // RG_GB
        {true, false}  // GG_RB
    };


    bool DetermineRatioState(void);
    void CalculateRelativeRatioStrength(double ratio, double &c0, double &c1, uint8_t bps);
    NormalisedWbsRegion ConvertDoubleRatiosToNormalisedResult(double R_G_frac, double B_G_frac);
    void ConvertRawToDoubleRatios(intel_vvp_wbs_zone_result raw, double &R_G_frac, double &B_G_frac);
    void ConvertWideRawToDoubleRatios(intel_vvp_wbs_zone_result_wide raw, double &R_G_frac, double &B_G_frac);
    void AsyncRead();

    // These give the best results generally
    WbsResultFormat _optimalFormats[4] =
    {
        GG_RB, // RGGB
        GG_RB, // GRBG
        GG_RB, // GBRG
        GG_RB // BGGR
    };
};

} // namespace SwApi
