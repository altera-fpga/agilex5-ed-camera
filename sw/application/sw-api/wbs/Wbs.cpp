/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Wbs.h"
#include "intel_vvp_wbs.h"
#include <unistd.h>

namespace SwApi {

std::shared_ptr<Wbs> Wbs::Create(Hapi::VvpWbsPtr spWbs)
{
    return std::make_shared<Wbs>(spWbs);
}

Wbs::Wbs(Hapi::VvpWbsPtr spWbs):
    VvpCoreBase("WBS"),
    _spWbs(spWbs)
{
}

void Wbs::WaitForPendingRead()
{
    // Wait for read to finish before applying settings
    while (_read_pending.load(std::memory_order_relaxed))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

bool Wbs::SetBypass(bool bypass, UpdatePolicy policy)
{
    auto update_hw = [instance = _spWbs->GetInstance(), bypass]()->bool {
        int rv = intel_vvp_wbs_set_bypass(instance, bypass);
        return (rv == kIntelVvpCoreOk || rv == kIntelVvpWbsCommitPendingErr);
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to set bypass\n";

    return ret;
}

bool Wbs::GetBypass()
{
    return intel_vvp_wbs_get_bypass(_spWbs->GetInstance());
}

bool Wbs::SetStatsFreeze(bool freeze)
{
    return intel_vvp_wbs_set_freeze_stats_request(_spWbs->GetInstance(), freeze) == kIntelVvpCoreOk;
}

bool Wbs::GetStatsFreezeSetting()
{
    return intel_vvp_wbs_get_freeze_stats_request(_spWbs->GetInstance());
}

bool Wbs::IsRunning()
{
    return intel_vvp_wbs_is_running(_spWbs->GetInstance());
}

bool Wbs::DetermineRatioState()
{
    // Wait for read to finish before applying settings
    WaitForPendingRead();

    bool cfaRatiosInverted = static_cast<uint8_t>(_cfaPhase) & 0x2;
    _swapRedBlue = (static_cast<uint8_t>(_cfaPhase) & 0x1);

    if (cfaRatiosInverted)
    {
        _invertRed  = !_invertLUT[_resultFormat][(_swapRedBlue) ? (1) : (0)];
        _invertBlue = !_invertLUT[_resultFormat][(_swapRedBlue) ? (0) : (1)];
    }
    else
    {
        _invertRed  = _invertLUT[_resultFormat][(_swapRedBlue) ? (1) : (0)];
        _invertBlue = _invertLUT[_resultFormat][(_swapRedBlue) ? (0) : (1)];
    }

    uint8_t invmask = ((_invertRed) ? (0x1) : (0)) | ((_invertBlue) ? (0x2) : (0));

    int rv = intel_vvp_wbs_set_cfa_invert(_spWbs->GetInstance(), invmask);

    return (rv == kIntelVvpCoreOk || rv == kIntelVvpWbsCommitPendingErr);
}

bool Wbs::SetResolution(uint32_t width, uint32_t height)
{
    bool ret = true;

    ret = ret && intel_vvp_core_set_img_info_width(_spWbs->GetInstance(), width) == kIntelVvpCoreOk;
    ret = ret && intel_vvp_core_set_img_info_height(_spWbs->GetInstance(), height) == kIntelVvpCoreOk;

    if(ret)
    {
        _width = width;
        _height = height;
    }

    return ret;
}

std::pair<uint32_t, uint32_t> Wbs::GetResolution()
{
    return {_width, _height};
}

uint16_t Wbs::GetPrecisionBits()
{
    return intel_vvp_wbs_get_precision_bits(_spWbs->GetInstance());
}

bool Wbs::SetCfaPhase(TCfaPhase cfaPhase, UpdatePolicy policy)
{
    auto update_hw = [this, cfaPhase]()->bool {

        WaitForPendingRead();

        int rv = intel_vvp_wbs_set_cfa_phase(_spWbs->GetInstance(), static_cast<uint8_t>(cfaPhase));
        bool ret = (rv == kIntelVvpCoreOk || rv == kIntelVvpWbsCommitPendingErr);

        if(ret)
        {
            const auto cfaBackup = _cfaPhase;
            _cfaPhase = cfaPhase;

            if(!DetermineRatioState())
            {
                _cfaPhase = cfaBackup;
                intel_vvp_wbs_set_cfa_phase(_spWbs->GetInstance(), static_cast<uint8_t>(cfaBackup));
                ret = false;
            }       
        }

        return ret;
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to set CFA phase\n";

    return ret;
}

TCfaPhase Wbs::GetCfaPhase()
{
    return _cfaPhase;
}

bool Wbs::SetResultFormat(WbsResultFormat format, UpdatePolicy policy)
{
    auto update_hw = [this, format]()->bool {
        WbsResultFormat bak = _resultFormat;
        _resultFormat = format;

        if(!DetermineRatioState())
        {
            _resultFormat = bak;
            return false;
        }

        return true;
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to set result format\n";
    
    return ret;
}

WbsResultFormat Wbs::GetResultFormat()
{
    return _resultFormat;
}

WbsRoI Wbs::GetClosestValidIdealRoI(WbsRoI& roi)
{
    WbsRoI idealRoI;

    // TODO - adjust depending on pip. This will suffice as a worst-case
    // for now.
    idealRoI.h_start = roi.h_start & (~0xf);
    idealRoI.v_start = roi.v_start & (~0xf);

    uint32_t width = (roi.h_end - idealRoI.h_start) + 1;
    uint32_t height = (roi.v_end - idealRoI.v_start) + 1;

    uint32_t width_num_blocks = ((width - 1) / 14);
    uint32_t height_num_blocks = ((height - 1) / 14);

    uint32_t adjustedXEnd = idealRoI.h_start + (width_num_blocks * 14) - 1;
    uint32_t adjustedYEnd = idealRoI.v_start + (height_num_blocks * 14) - 1;

    if (adjustedXEnd > (_width-1))
    {
        adjustedXEnd = _width - 1;
    }

    if (adjustedYEnd > (_height-1))
    {
        adjustedYEnd = _height - 1;
    }


    idealRoI.h_end = idealRoI.h_start + (14 * width_num_blocks) - 1;
    idealRoI.v_end = idealRoI.v_start + (14 * height_num_blocks) - 1;

    return idealRoI;
}

bool Wbs::SetRoI(WbsRoI& roi, UpdatePolicy policy)
{
    WbsRoI idealRoI = GetClosestValidIdealRoI(roi);

    auto update_hw = [this, idealRoI]()->bool {
        WaitForPendingRead();

        int rv[] = {
            intel_vvp_wbs_set_h_start(_spWbs->GetInstance(), idealRoI.h_start),
            intel_vvp_wbs_set_v_start(_spWbs->GetInstance(), idealRoI.v_start),
            intel_vvp_wbs_set_h_end(_spWbs->GetInstance(), idealRoI.h_end + 1), // RTL requires the end coords be +1
            intel_vvp_wbs_set_v_end(_spWbs->GetInstance(), idealRoI.v_end + 1),        
            intel_vvp_wbs_set_zone_h_count(_spWbs->GetInstance(), ((idealRoI.h_end - idealRoI.h_start) + 1) / 7),
            intel_vvp_wbs_set_zone_v_count(_spWbs->GetInstance(), ((idealRoI.v_end - idealRoI.v_start) + 1) / 7)
        };

        return ((rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpWbsCommitPendingErr)) &&
                ((rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpWbsCommitPendingErr)) &&
                ((rv[2] == kIntelVvpCoreOk || rv[2] == kIntelVvpWbsCommitPendingErr)) &&
                ((rv[3] == kIntelVvpCoreOk || rv[3] == kIntelVvpWbsCommitPendingErr)) &&
                ((rv[4] == kIntelVvpCoreOk || rv[4] == kIntelVvpWbsCommitPendingErr)) &&
                ((rv[5] == kIntelVvpCoreOk || rv[5] == kIntelVvpWbsCommitPendingErr));
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to set Region of Interest\n";

    return ret;
}

WbsRoI Wbs::GetRoI(void)
{
    WbsRoI readRoI;

    readRoI.h_start = intel_vvp_wbs_get_h_start(_spWbs->GetInstance());
    readRoI.h_end = intel_vvp_wbs_get_h_end(_spWbs->GetInstance());
    readRoI.v_start = intel_vvp_wbs_get_v_start(_spWbs->GetInstance());
    readRoI.v_end = intel_vvp_wbs_get_v_end(_spWbs->GetInstance());

    return readRoI;
}

bool Wbs::SetCfaX0Ranges(uint32_t upper, uint32_t lower, UpdatePolicy policy)
{
    auto update_hw = [this, upper, lower]()->bool {
        // Wait for read to finish before applying settings
        WaitForPendingRead();

        int rv[] = {
            intel_vvp_wbs_set_cfa_x0_range_lo(_spWbs->GetInstance(), lower),
            intel_vvp_wbs_set_cfa_x0_range_hi(_spWbs->GetInstance(), upper)
        };

        return (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpWbsCommitPendingErr) &&
               (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpWbsCommitPendingErr);
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to set CFA X0 ranges\n";

    return ret;
}

bool Wbs::SetCfaX0Ranges(float upper, float lower, UpdatePolicy policy)
{
    uint32_t upper_fixed = ((uint16_t) upper << 16) + ((upper - ((uint16_t) upper)) * ((1 << 16) - 1));
    uint32_t lower_fixed = ((uint16_t) lower << 16) + ((lower - ((uint16_t) lower)) * ((1 << 16) - 1));

    return SetCfaX0Ranges(upper_fixed, lower_fixed, policy);
}

bool Wbs::GetCfaX0Ranges(uint32_t &upper, uint32_t &lower)
{
    upper = intel_vvp_wbs_get_cfa_x0_range_hi(_spWbs->GetInstance());
    lower = intel_vvp_wbs_get_cfa_x0_range_lo(_spWbs->GetInstance());

    return true; // FIXME error check
}

bool Wbs::GetCfaX0Ranges(float &upper, float &lower)
{
    uint32_t upperOut;
    uint32_t lowerOut;

    GetCfaX0Ranges(upperOut, lowerOut);

    upper = (float)((upperOut) >> 16) + ((float)(upperOut & 0xFFFF) / (1 << 16));
    lower = (float)((lowerOut) >> 16) + ((float)(lowerOut & 0xFFFF) / (1 << 16));

    return true; // FIXME error check
}

bool Wbs::SetCfaX1Ranges(uint32_t upper, uint32_t lower, UpdatePolicy policy)
{
    auto update_hw = [this, upper, lower]()->bool {
        // Wait for read to finish before applying settings
        WaitForPendingRead();

        int rv[] = {
            intel_vvp_wbs_set_cfa_x1_range_lo(_spWbs->GetInstance(), lower),
            intel_vvp_wbs_set_cfa_x1_range_hi(_spWbs->GetInstance(), upper)
        };

        return (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpWbsCommitPendingErr) &&
               (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpWbsCommitPendingErr);
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to set CFA X1 ranges\n";

    return ret;
}

bool Wbs::SetCfaX1Ranges(float upper, float lower, UpdatePolicy policy)
{
    uint32_t upper_fixed = ((uint16_t) upper << 16) + ((upper - ((uint16_t) upper)) * ((1 << 16) - 1));
    uint32_t lower_fixed = ((uint16_t) lower << 16) + ((lower - ((uint16_t) lower)) * ((1 << 16) - 1));

    return SetCfaX1Ranges(upper_fixed, lower_fixed, policy);
}

bool Wbs::GetCfaX1Ranges(uint32_t &upper, uint32_t &lower)
{
    upper = intel_vvp_wbs_get_cfa_x1_range_hi(_spWbs->GetInstance());
    lower = intel_vvp_wbs_get_cfa_x1_range_lo(_spWbs->GetInstance());

    return true; // FIXME error check
}

bool Wbs::GetCfaX1Ranges(float &upper, float &lower)
{
    uint32_t upperOut;
    uint32_t lowerOut;

    GetCfaX1Ranges(upperOut, lowerOut);

    upper = (float)((upperOut) >> 16) + ((float)(upperOut & 0xFFFF) / (1 << 16));
    lower = (float)((lowerOut) >> 16) + ((float)(lowerOut & 0xFFFF) / (1 << 16));

    return true; // FIXME error check
}

void Wbs::CalculateRelativeRatioStrength(double ratio, double &c0, double &c1, uint8_t bps)
{
    // Strength guesstimating
    if (ratio > 1.0)
    {
        // If ratio > 1, then c0 is larger than c1
        c1 = (1 / ratio) / 2;
        c0 = 1.0 - c1;
    }
    else if (ratio < 1.0)
    {
        // If ratio <= 1, then c1 is larger than c0
        c0 = ratio / 2;
        c1 = 1.0 - c0;
    }
    else
    {
        // If ratio == 1, then c0 == c1
        c0 = 0.5;
        c1 = 0.5;
    }
}

void Wbs::ConvertRawToDoubleRatios(intel_vvp_wbs_zone_result raw, double &R_G_frac, double &B_G_frac)
{
    uint8_t frac_bits = intel_vvp_wbs_get_precision_bits(_spWbs->GetInstance());

    if (_swapRedBlue)
    {
        R_G_frac = raw.x1_integer + ((double)raw.x1_fraction / (double)(1 << frac_bits));
        B_G_frac = raw.x0_integer + ((double)raw.x0_fraction / (double)(1 << frac_bits));
    }
    else
    {
        R_G_frac = raw.x0_integer + ((double)raw.x0_fraction / (double)(1 << frac_bits));
        B_G_frac = raw.x1_integer + ((double)raw.x1_fraction / (double)(1 << frac_bits));
    }

    // Get the average ratio
    // This can be either a 0-1 normalised value, or anything up to bps in size
    // depending on result format
    R_G_frac /= raw.num_pixels_accumulated;
    B_G_frac /= raw.num_pixels_accumulated;
}

NormalisedWbsRegion Wbs::ConvertDoubleRatiosToNormalisedResult(double R_G_frac, double B_G_frac)
{
    NormalisedWbsRegion normalisedResult;

    uint8_t bps = intel_vvp_wbs_get_bits_per_sample_in(_spWbs->GetInstance());

    double R_strength;
    double RG_strength;
    double B_strength;
    double BG_strength;

    switch (_resultFormat)
    {
        case WbsResultFormat::RB_GG:
        {
            CalculateRelativeRatioStrength(R_G_frac, R_strength, RG_strength, bps);
            CalculateRelativeRatioStrength(B_G_frac, B_strength, BG_strength, bps);
            break;
        }
        case WbsResultFormat::GB_RG:
        {
            CalculateRelativeRatioStrength(R_G_frac, RG_strength, R_strength, bps);
            CalculateRelativeRatioStrength(B_G_frac, B_strength,  BG_strength, bps);
            break;
        }
        case WbsResultFormat::RG_GB:
        {
            CalculateRelativeRatioStrength(R_G_frac, R_strength,  RG_strength, bps);
            CalculateRelativeRatioStrength(B_G_frac, BG_strength, B_strength, bps);
            break;
        }
        case WbsResultFormat::GG_RB:
        {
            CalculateRelativeRatioStrength(R_G_frac, RG_strength, R_strength, bps);
            CalculateRelativeRatioStrength(B_G_frac, BG_strength, B_strength, bps);
            break;
        }
        default:
            break;
    }

    double Red_Green_Multiples = (R_strength / RG_strength);
    double Blue_Green_Multiples = (B_strength / BG_strength);
    double Green_Multiples = 1;

    double maxRange = Red_Green_Multiples + Blue_Green_Multiples + Green_Multiples;

    normalisedResult.red_strength = Red_Green_Multiples / maxRange;
    normalisedResult.green_strength = Green_Multiples / maxRange;
    normalisedResult.blue_strength = Blue_Green_Multiples / maxRange;

    return normalisedResult;
}

NormalisedWbsRegion Wbs::ConvertRawToNormalisedResult(intel_vvp_wbs_zone_result raw)
{
    if (!raw.num_pixels_accumulated)
    {
        NormalisedWbsRegion normalisedResult;
        normalisedResult.red_strength = 0.0f;
        normalisedResult.green_strength = 0.0f;
        normalisedResult.blue_strength = 0.0f;
        return normalisedResult;
    }

    double R_G_frac;
    double B_G_frac;

    ConvertRawToDoubleRatios(raw, R_G_frac, B_G_frac);


    return ConvertDoubleRatiosToNormalisedResult(R_G_frac, B_G_frac);
}

void Wbs::ConvertWideRawToDoubleRatios(intel_vvp_wbs_zone_result_wide raw, double &R_G_frac, double &B_G_frac)
{
    uint8_t frac_bits = intel_vvp_wbs_get_precision_bits(_spWbs->GetInstance());

    if (_swapRedBlue)
    {
        R_G_frac = raw.x1_integer + ((double)raw.x1_fraction / (double)(1 << frac_bits));
        B_G_frac = raw.x0_integer + ((double)raw.x0_fraction / (double)(1 << frac_bits));
    }
    else
    {
        R_G_frac = raw.x0_integer + ((double)raw.x0_fraction / (double)(1 << frac_bits));
        B_G_frac = raw.x1_integer + ((double)raw.x1_fraction / (double)(1 << frac_bits));
    }

    // Get the average ratio
    // This can be either a 0-1 normalised value, or anything up to bps in size
    // depending on result format
    R_G_frac /= raw.num_pixels_accumulated;
    B_G_frac /= raw.num_pixels_accumulated;
}

NormalisedWbsRegion Wbs::ConvertWideRawToNormalisedResult(intel_vvp_wbs_zone_result_wide raw)
{
    if (!raw.num_pixels_accumulated)
    {
        NormalisedWbsRegion normalisedResult;
        normalisedResult.red_strength = 0.0f;
        normalisedResult.green_strength = 0.0f;
        normalisedResult.blue_strength = 0.0f;
        return normalisedResult;
    }

    double R_G_frac;
    double B_G_frac;

    ConvertWideRawToDoubleRatios(raw, R_G_frac, B_G_frac);

    return ConvertDoubleRatiosToNormalisedResult(R_G_frac, B_G_frac);
}

bool Wbs::StatsFreezeHandshake(void)
{
    // TODO - replace this with a sub thread rather than stalling the main thread
    intel_vvp_wbs_set_freeze_stats_request(_spWbs->GetInstance(), false);
    usleep(50000); // Wait a ~4 fps frame, should be a reasonable window.
    for (int i = 0; i < 5; i++)
    {
        if (!intel_vvp_wbs_stats_are_frozen(_spWbs->GetInstance()))
        {
            return true;
        }
        usleep(50000);
    }
    std::cout << "Handshake has failed, stats_frozen status bit not set" << std::endl;
    intel_vvp_wbs_set_freeze_stats_request(_spWbs->GetInstance(), true);
    return false;
}

WbsResults Wbs::ReadResultTable(void)
{
    if (_read_pending.load(std::memory_order_relaxed))
    {
        // Wait for read to finish
        while (_read_pending.load(std::memory_order_relaxed))
        {
            usleep(1000);
        }

        std::atomic_thread_fence(std::memory_order_relaxed);
    }
    else
    {
        _read_pending.store(true, std::memory_order_relaxed);
        AsyncRead();
        std::atomic_thread_fence(std::memory_order_relaxed);
        _read_pending.store(false, std::memory_order_relaxed);
    }

    return _results;
}

void Wbs::AsyncRead()
{
    if (intel_vvp_wbs_stats_are_frozen(_spWbs->GetInstance()))
    {
        if (!StatsFreezeHandshake())
        {
            _results = {0};
        }
    }

    for (uint32_t y = 0; y < 7; y++)
    {
        for (uint32_t x = 0; x < 7; x++)
        {
            _results.raw[y][x] = intel_vvp_wbs_get_formatted_table_result(_spWbs->GetInstance(), x, y);
            _results.normalised[y][x] = ConvertRawToNormalisedResult(_results.raw[y][x]);
        }
    }

    intel_vvp_wbs_set_freeze_stats_request(_spWbs->GetInstance(), true);
}

WbsResultFormat Wbs::GetOptimalResultFormatForCurrentSetup()
{
    return _optimalFormats[static_cast<uint8_t>(_cfaPhase)];
}

bool Wbs::CommitSettings()
{
    return (intel_vvp_wbs_commit(_spWbs->GetInstance()) == kIntelVvpCoreOk);
}

bool Wbs::IsCommitPending()
{
    return intel_vvp_wbs_commit_is_pending(_spWbs->GetInstance());
}

} // namespace SwApi
