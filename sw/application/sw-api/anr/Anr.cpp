/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "Anr.h"
#include "DefaultLuts.h"
#include <cmath>
#include <iostream>

#include <cmath>


namespace SwApi {


static constexpr uint32_t TF32_SHIFT = 13;


std::shared_ptr<Anr> Anr::Create(Hapi::VvpAnrPtr spAnr, uint32_t initialOutputWidth, uint32_t initialOutputHeight)
{
    return std::make_shared<Anr>(spAnr, initialOutputWidth, initialOutputHeight);
}


Anr::Anr(Hapi::VvpAnrPtr spAnr, uint32_t initialOutputWidth, uint32_t initialOutputHeight):
    VvpCoreBase("ANR"),
    _spAnr(spAnr),
    _bypass{false}
{
    // Set the output width and height.
    SetResolution(initialOutputWidth, initialOutputHeight);

    int cfaDivider = (intel_vvp_anr_get_cfa_enabled(_spAnr->GetInstance())) ? (4) : (2);

    _spatialLutXDepth = (intel_vvp_anr_get_num_h_taps(_spAnr->GetInstance()) - 1) / cfaDivider;
    _spatialLutYDepth = (intel_vvp_anr_get_num_v_taps(_spAnr->GetInstance()) - 1) / cfaDivider;
}

bool Anr::SetBypass(bool bypass, UpdatePolicy policy)
{
    auto update_hw = [this, bypass]()->bool {
        int rv = intel_vvp_anr_set_bypass(_spAnr->GetInstance(), bypass);
        return (rv == kIntelVvpCoreOk || rv == kIntelVvpAnrCommitPendingErr);
    };

    bool ret = UpdateHw(update_hw, policy);

    if(ret)
        _bypass = bypass;
    else
        std::cerr << "[" << GetName() << "] Failed change bypass setting" << std::endl;

    return ret;
}

bool Anr::GetBypass(void)
{
    return intel_vvp_anr_get_bypass(_spAnr->GetInstance());
}

bool Anr::SetResolution(uint32_t width, uint32_t height)
{
    bool ret = true;

    ret = ret && (intel_vvp_core_set_img_info_width(_spAnr->GetInstance(), width) == kIntelVvpCoreOk);
    ret = ret && (intel_vvp_core_set_img_info_height(_spAnr->GetInstance(), height) == kIntelVvpCoreOk);

    return ret;
}

bool Anr::ApplyUnityLuts()
{
    _currentSpatialLut = SwApi::AnrLuts::GetUnitySpatialLut(_spatialLutXDepth + _spatialLutYDepth);
    _currentIntensityLut = SwApi::AnrLuts::GetUnityIntensityLut();

    return ApplyLuts();
}

bool Anr::ApplyLuts(float combinedGain, float darkNoise, float intensityStrength, float spatialRadiusScaler, UpdatePolicy policy)
{
    _currentCombinedGain = combinedGain;
    _currentDarkNoise = darkNoise;
    if (intensityStrength != -1) _currentIntensityStrength = intensityStrength;
    if (spatialRadiusScaler != -1) _currentSpatialRadiusScaler = spatialRadiusScaler;

    GenerateSpatialLut();
    GenerateIntensityLut();

    return ApplyLuts(policy);
}

bool Anr::ApplyLuts(float intensityStrength, UpdatePolicy policy)
{
    _currentIntensityStrength = intensityStrength;

    GenerateIntensityLut();

    return ApplyLuts(policy);
}

bool Anr::ApplyLuts(UpdatePolicy policy)
{
    auto update_hw = [this]()->bool {

        const auto instance = _spAnr->GetInstance();

        int rv[] = {
            intel_vvp_anr_write_intensity_lut_array(instance, _currentIntensityLut.data()),
            intel_vvp_anr_write_spatial_lut_array(instance, _currentSpatialLut.data()),
            intel_vvp_anr_set_bypass(instance, _bypass)
        };

        bool ret = (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpAnrCommitPendingErr);
        ret = ret && (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpAnrCommitPendingErr);
        ret = ret && (rv[2] == kIntelVvpCoreOk || rv[2] == kIntelVvpAnrCommitPendingErr);

        return ret;
    };

    bool ret = UpdateHw(update_hw, policy);

    if(!ret)
        std::cerr << "[" << GetName() << "] Failed to apply LUTs" << std::endl;

    return ret;
}

void Anr::GenerateSpatialLut()
{
    float scaler = std::max(_currentSpatialRadiusScaler, 1e-6f);
    const bool cfaEnabled = intel_vvp_anr_get_cfa_enabled(_spAnr->GetInstance());

    _currentSpatialLut.clear();
    _currentSpatialLut.reserve(_spatialLutXDepth + _spatialLutYDepth);

    // Horizontal
    const uint8_t hTaps = intel_vvp_anr_get_num_h_taps(_spAnr->GetInstance());
    float hRadius = scaler * static_cast<float>(hTaps) / 2;
    const uint8_t hItr = (cfaEnabled) ? (2) : (1);

    for (int itr = 0; itr < _spatialLutXDepth; itr++)
    {
        Anr::ViewableFloat spatial_entry;
        const float tmp = static_cast<float>(hItr * (itr + 1)) / static_cast<float>(hRadius);
        spatial_entry.f = ((tmp * tmp) * 0.5f);
        _currentSpatialLut.push_back(spatial_entry.ui >> TF32_SHIFT);
    }

    // Vertical
    const uint8_t vTaps = intel_vvp_anr_get_num_v_taps(_spAnr->GetInstance());
    float vRadius = scaler * static_cast<float>(vTaps) / 2;
    const uint8_t vItr = (cfaEnabled) ? (2) : (1);

    for (int itr = 0; itr < _spatialLutYDepth; itr++)
    {
        Anr::ViewableFloat spatial_entry;
        const float tmp = static_cast<float>(vItr * (itr + 1)) / static_cast<float>(vRadius);
        spatial_entry.f = ((tmp * tmp) * 0.5f);
        _currentSpatialLut.push_back(spatial_entry.ui >> TF32_SHIFT);
    }
}

void Anr::GenerateIntensityLut()
{
    float scaler = std::max(_currentIntensityStrength, 1e-6f);
    uint8_t bps = intel_vvp_anr_get_bits_per_sample_in(_spAnr->GetInstance());
    uint32_t increment = bps > 10 ? (1 << bps) / INTEL_VVP_ANR_INTENSITY_LUT_ENTRIES : 1;
    const float darkNoiseWithGainSquared = scaler * _currentDarkNoise * _currentDarkNoise;
    const float combinedGainSquared = scaler * _currentCombinedGain;

    _currentIntensityLut.clear();
    _currentIntensityLut.reserve(INTEL_VVP_ANR_INTENSITY_LUT_ENTRIES);

    for (int i = 0; i < INTEL_VVP_ANR_INTENSITY_LUT_ENTRIES; i++)
    {
        Anr::ViewableFloat intensity_entry;
        intensity_entry.f = sqrtf(1.0f / (2.0f * ((static_cast<float>(i * increment) * combinedGainSquared) + darkNoiseWithGainSquared)));
        _currentIntensityLut.push_back(intensity_entry.ui >> TF32_SHIFT);
    }
}

bool Anr::CommitSettings()
{
    return (intel_vvp_anr_commit(_spAnr->GetInstance()) == kIntelVvpCoreOk);
}

bool Anr::IsCommitPending()
{
    return intel_vvp_anr_commit_is_pending(_spAnr->GetInstance());
}

} // namespace SwApi