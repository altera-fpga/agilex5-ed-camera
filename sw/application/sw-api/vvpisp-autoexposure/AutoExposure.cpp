/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "AutoExposure.h"
#include "CoeffGen.h"
#include <cmath>
#include <algorithm>


AutoExposureController::AutoExposureController(const std::shared_ptr<SwApi::HistogramStats>& spHs, 
                                                const std::shared_ptr<SwApi::Ccm>& spCcm)
    :_spHs(spHs),
    _spCcm(spCcm),
    _spCamera(nullptr),
    _spMultiChannelAutoExposure(nullptr),
    _multiChannelAutoExposureHandle(0),
    _roi{0,0,0,0},
    _previous_roi{0,0,0,0},
    _frame_index{0},
    _previous_mults{1.0f, 1.0f, 1.0f},
    _damping_index{0},
    _last_d2_sign{true} // Initialisation state doesn't really matter, only affects the first loop
{
    for (int i = 0; i < FRAMES_AVERAGED; i++)
    {
        _tableValues[i] = std::make_shared<SwApi::Hs::TableResults>();
        _tableValues[i]->frame_luma_bins = {0, {0}};
        _tableValues[i]->roi_luma_bins = {0, {0}};
        _tableValues[i]->valid = false;
    }

    if (_spHs)
    {
        _histogramSubscriptionId = _spHs->RegisterHistogramObserver([this](const std::shared_ptr<SwApi::Hs::TableResults>& histogramData)
        {
            // When new histogram arrives
            // insert it into the table and update frame pointer and counter
            if(histogramData->valid)
            {
                std::scoped_lock lock(_histogramDataMutex);
                _frame_index = (_frame_index + 1) % FRAMES_AVERAGED;
                _tableValues[_frame_index] = histogramData;
                ++_totalFrames;
            }
        });
    }    
}


AutoExposureController::~AutoExposureController()
{
    if (_spHs && _histogramSubscriptionId != 0)
    {
        _spHs->UnregisterHistogramObserver(_histogramSubscriptionId);
    }
}


void AutoExposureController::SetCamera(ICameraPtr spCamera)
{
    _spCamera = std::move(spCamera);
}


void AutoExposureController::SetMultiChannelAutoExposure(IMultiChannelAutoExposurePtr& spMultiChannelAutoExposure, uint32_t handle)
{
    _spMultiChannelAutoExposure = spMultiChannelAutoExposure;
    _multiChannelAutoExposureHandle = handle;
}


SwApi::Hs::TableResults AutoExposureController::GetLumaBins()
{
    SwApi::Hs::TableResults tableAverages{};

    std::scoped_lock lock(_histogramDataMutex);

    uint32_t frames_used = std::min(_totalFrames, FRAMES_AVERAGED);
    
    for (uint32_t i = 0; i <  _tableValues[_frame_index]->frame_luma_bins.num_luma_bins; i++)
    {
        uint32_t frame_total = 0;
        uint32_t roi_total = 0;

        // Average the luma across multiple frames
        for (uint32_t j = 0; j < frames_used; j++)
        {
            const uint32_t idx = (_frame_index + FRAMES_AVERAGED - j) % FRAMES_AVERAGED;
            frame_total += _tableValues[idx]->frame_luma_bins.luma_bins[i];
            roi_total += _tableValues[idx]->roi_luma_bins.luma_bins[i];
        }

        tableAverages.frame_luma_bins.luma_bins[i] = frame_total / frames_used;
        tableAverages.roi_luma_bins.luma_bins[i] = roi_total / frames_used;
    }

    tableAverages.valid = _tableValues[_frame_index]->valid;
    tableAverages.frame_luma_bins.num_luma_bins = _tableValues[0]->frame_luma_bins.num_luma_bins;
    tableAverages.roi_luma_bins.num_luma_bins = _tableValues[0]->roi_luma_bins.num_luma_bins;

    if (tableAverages.frame_luma_bins.num_luma_bins != tableAverages.roi_luma_bins.num_luma_bins)
        std::cout << "[AE] Warning: Frame and RoI have different numbers of bins" << std::endl;

    return tableAverages;
}

void AutoExposureController::SetThresholds(float underThreshold, float satThreshold)
{
    _underThreshold = underThreshold;
    _satThreshold = satThreshold;
}

std::pair<float, float> AutoExposureController::GetThresholds()
{
    return {_underThreshold, _satThreshold};
}

void AutoExposureController::AdjustThresholds(const SwApi::Hs::TableResults& luma_bins)
{
    const auto [w, h] = GetInputResolution();

    unsigned int total_pixel_count = w * h;
    unsigned int num_bins = luma_bins.frame_luma_bins.num_luma_bins;

    // Contract thresholds if BOTH the upper and lower edges are below a certain level
    // i.e. the distribution is fully contained within the thresholds

    // Spread thresholds if EITHER the upper or lower edges are above a certain level
    // i.e. a significant part of the distribution is outside the thresholds

    float outside_min = 0.05;
    float outside_max = 0.25;

    unsigned int under_total = 0;
    unsigned int over_total = 0;
    for (int i = 0; i < _underThreshold * num_bins; i++)
    {
        under_total += luma_bins.frame_luma_bins.luma_bins[i];
    }
    for (int i = num_bins - 1; i > _satThreshold * num_bins; i--)
    {
        over_total += luma_bins.frame_luma_bins.luma_bins[i];
    }

    float under_proportion = (float)under_total / total_pixel_count;
    float over_proportion = (float)over_total / total_pixel_count;

    if (under_proportion < outside_min && over_proportion < outside_min)
    {
        // Contract thresholds
        if (_underThreshold < 0.35)
        {
            _underThreshold += 0.01;
        }
        if (_satThreshold > 0.65)
        {
            _satThreshold -= 0.01;
        }
    }
    else if (under_proportion > outside_max && over_proportion > outside_max)
    {
        // Spread thresholds
        if (_underThreshold > 0.06)
        {
            _underThreshold -= 0.01;
        }
        if (_satThreshold < 0.94)
        {
            _satThreshold += 0.01;
        }
    }

    if (under_proportion > outside_max && over_proportion < outside_max)
    {
        if (_underThreshold > 0.06)
        {
            _underThreshold -= 0.01;
        }
    }

    if (under_proportion < outside_min && over_proportion > outside_min)
    {
        if (_underThreshold < 0.35)
        {
            _underThreshold += 0.01;
        }
    }

    if (over_proportion > outside_max && under_proportion < outside_max)
    {
        if (_satThreshold < 0.94)
        {
            _satThreshold += 0.01;
        }
    }

    if (over_proportion < outside_min && under_proportion > outside_min)
    {
        if (_satThreshold > 0.65)
        {
            _satThreshold -= 0.01;
        }
    }
}

float AutoExposureController::CalculateExposureMult(const SwApi::Hs::TableResults& luma_bins, const SwApi::Hs::RegionOfInterest& roi)
{
    if (luma_bins.frame_luma_bins.num_luma_bins == 0)
        return 1.0f;

    if (luma_bins.frame_luma_bins.num_luma_bins != luma_bins.roi_luma_bins.num_luma_bins)
        return 1.0f;

    auto num_luma_bins = luma_bins.frame_luma_bins.num_luma_bins;

    const auto [w, h] = GetInputResolution();

    unsigned int image_pixel_count = w * h;
    unsigned int roi_width = std::clamp(static_cast<uint32_t>(roi.h_end - roi.h_start), 1U, w);
    unsigned int roi_height = std::clamp(static_cast<uint32_t>(roi.v_end - roi.v_start), 1U, h);
    unsigned int roi_pixel_count = roi_width * roi_height;

    float exp_mult = 1.0f;

    ///////////////
    // ENTIRE IMAGE
    ///////////////

    // Mean/Auto
    uint64_t auto_total = 0;
    uint64_t count = 0;
    for (unsigned int bin = 0; bin < num_luma_bins; ++bin)
    {
        auto_total += static_cast<uint64_t>(luma_bins.frame_luma_bins.luma_bins[bin]) * bin;
        count += static_cast<uint64_t>(luma_bins.frame_luma_bins.luma_bins[bin]);
    }
    float mean = (float)auto_total / (image_pixel_count * num_luma_bins);
    if(_spMultiChannelAutoExposure != nullptr)
    {
        _spMultiChannelAutoExposure->BalanceMean(_multiChannelAutoExposureHandle, mean);
    }

    float image_mean_mult = powf(fabs(mean - _meanBrightness) * _meanBrightnessStrength, _mean_brightness_convergence);
    if (mean > _meanBrightness)
    {
        image_mean_mult = 1.0f - image_mean_mult;
    }
    else
    {
        image_mean_mult = 1.0f + image_mean_mult;
    }


    // Sat
    uint64_t sat_total = 0;
    for (unsigned int bin = num_luma_bins - 1; bin > num_luma_bins * _satThreshold; --bin)
    {
        sat_total += luma_bins.frame_luma_bins.luma_bins[bin];
    }

    float image_sat_mult = 1;
    float sat_proportion = (float)sat_total / image_pixel_count;
    if(_spMultiChannelAutoExposure != nullptr)
    {
        _spMultiChannelAutoExposure->BalanceSaturation(_multiChannelAutoExposureHandle, sat_proportion);
    }
    if (sat_proportion > _sat_tolerance)
    {
        image_sat_mult = powf(1.0f - (sat_proportion - _sat_tolerance) * _desaturationStrength, _desaturation_convergence);
    }
    if (image_sat_mult < _min_sat_mult) image_sat_mult = _min_sat_mult;

    // Under
    uint64_t under_total = 0;
    for (unsigned int bin = 0; bin < num_luma_bins * _underThreshold; ++bin)
    {
        under_total += luma_bins.frame_luma_bins.luma_bins[bin];
    }

    float image_under_mult = 1.0f;
    float under_proportion = (float)under_total / image_pixel_count;
    if(_spMultiChannelAutoExposure != nullptr)
    {
        _spMultiChannelAutoExposure->BalanceUnderSaturation(_multiChannelAutoExposureHandle, under_proportion);
    }
    if (under_proportion > _under_tolerance)
    {
        image_under_mult = powf(1.0f + (under_proportion - _under_tolerance) * _shadowPreservationStrength, _shadow_preservation_convergence);
    }
    if (image_under_mult > _max_under_mult) image_under_mult = _max_under_mult;

    float image_mult = image_mean_mult * image_sat_mult * image_under_mult;

    if (_enableRoi)
    {
        ///////////////
        // ROI
        ///////////////

        // Mean/Auto
        auto_total = 0;

        for (unsigned int bin = 0; bin < num_luma_bins; ++bin)
        {
            auto_total += static_cast<uint64_t>(luma_bins.roi_luma_bins.luma_bins[bin]) * bin;
        }

        mean = (float)auto_total / (roi_pixel_count * num_luma_bins);
        float roi_mean_mult = powf(fabs(mean - _meanBrightness) * _meanBrightnessStrength, _mean_brightness_convergence);
        
        if (mean > _meanBrightness)
        {
            roi_mean_mult = 1 - roi_mean_mult;
        }
        else
        {
            roi_mean_mult = 1 + roi_mean_mult;
        }

        // Sat
        sat_total = 0;
        for (unsigned int bin = num_luma_bins - 1; bin > num_luma_bins * _satThreshold; --bin)
        {
            sat_total += luma_bins.roi_luma_bins.luma_bins[bin];
        }
        float roi_sat_mult = 1;
        sat_proportion = sat_total / roi_pixel_count;
        if (sat_proportion > _sat_tolerance)
        {
            roi_sat_mult = powf(1.0f - (sat_proportion - _sat_tolerance) * _desaturationStrength, _desaturation_convergence);
        }
        if (roi_sat_mult < _min_sat_mult) roi_sat_mult = _min_sat_mult;

        // Under
        under_total = 0;
        for (unsigned int bin = 0; bin < num_luma_bins * _underThreshold; ++bin)
        {
            under_total += luma_bins.roi_luma_bins.luma_bins[bin];
        }

        float roi_under_mult = 1.0f;
        under_proportion = under_total / roi_pixel_count;
        if (under_proportion > _under_tolerance)
        {
            roi_under_mult = pow(1 + (under_proportion - _under_tolerance) * _shadowPreservationStrength, _shadow_preservation_convergence);
        }

        if (roi_under_mult > _max_under_mult) roi_under_mult = _max_under_mult;

        float roi_mult = roi_mean_mult*roi_sat_mult*roi_under_mult;

        exp_mult = powf(image_mult, (1.0f - _roiRatio)) * powf(roi_mult, _roiRatio);
    }
    else
    {
        exp_mult = image_mult;
    }

    return exp_mult;
}

float AutoExposureController::CalculateShutterSpeedGain(float& gain)
{
    float shutter_speed = 0.0f;

    if(_spCamera)
    {
        const auto [min_shutter_speed, max_shutter_speed] = _spCamera->GetShutterSpeedRange();
        auto old_shutter_speed = _spCamera->GetShutterSpeed();

        float target_shutter_speed = old_shutter_speed * gain;

        if (target_shutter_speed > max_shutter_speed)
        {
            shutter_speed = max_shutter_speed;
            gain = target_shutter_speed / max_shutter_speed;
        }
        else if (target_shutter_speed < min_shutter_speed)
        {
            shutter_speed = min_shutter_speed;
            gain = target_shutter_speed / min_shutter_speed;
        }
        else
        {
            shutter_speed = target_shutter_speed;
            gain = 1.0f;
        }
    }

    return shutter_speed;
}

float AutoExposureController::CalculateAnalogueGain(float& gain)
{
    float analogue_gain = 0.0f;

    if(_spCamera)
    {
        const auto [analogue_gain_min, analogue_gain_max] = _spCamera->GetAnalogueGainRange();

        if (gain < analogue_gain_max && gain > analogue_gain_min)
        {
            analogue_gain = (uint32_t)(gain / _spCamera->GetAnalogueGainStep()) * _spCamera->GetAnalogueGainStep();
        }
        else if (gain <= analogue_gain_min)
        {
            analogue_gain = analogue_gain_min;
        }
        else if (gain >= analogue_gain_max)
        {
            analogue_gain = analogue_gain_max;
        }

        gain /= analogue_gain;
    }

    return analogue_gain;
}

float AutoExposureController::CalculateDigitalGain(float& gain)
{
    float digital_gain = 0.0f;

    if(_spCamera)
    {
        const auto [digital_gain_min, digital_gain_max] = _spCamera->GetDigitalGainRange();

        if (gain < digital_gain_max && gain > digital_gain_min)
        {
            digital_gain = (uint32_t)(gain / _spCamera->GetDigitalGainStep()) * _spCamera->GetDigitalGainStep();
        }
        else if (gain <= digital_gain_min)
        {
            digital_gain = digital_gain_min;
        }
        else if (gain >= digital_gain_max)
        {
            digital_gain = digital_gain_max;
        }

        gain /= digital_gain;
    }

    return digital_gain;
}

float AutoExposureController::CalculateDigitalCcmGain(float& gain)
{
    static constexpr float MIN_DIGITAL_CCM_GAIN = 1.0f; // Must not reduce brightness, creates artifacts
    static constexpr float MAX_DIGITAL_CCM_GAIN = 1.0f; // Max equal min disables CCM gain

    const float digital_gain = std::clamp(gain, MIN_DIGITAL_CCM_GAIN, MAX_DIGITAL_CCM_GAIN);

    gain /= digital_gain;
    return digital_gain;
}

void AutoExposureController::RunAutoExposure()
{
    if((_previous_roi.h_start != _roi.h_start) || (_previous_roi.v_start != _roi.v_start) ||
        (_previous_roi.h_end != _roi.h_end) || (_previous_roi.v_end != _roi.v_end))
    {
        _previous_roi = _roi;
        _spHs->SetRegionOfInterest(_previous_roi);
    }

    SwApi::Hs::TableResults luma_bins = GetLumaBins();

    if (_controlThresholds)
    {
        AdjustThresholds(luma_bins);
    }

    float exp_mult = CalculateExposureMult(luma_bins, _previous_roi);

    // Oscillation prevention
    if (_damping)
    {
        float d1 = exp_mult - _previous_mults[_damping_index];
        float d2 = _previous_mults[_damping_index] - _previous_mults[(_damping_index + 1) % 2];

        float d2_1 = d1 - d2;
        bool d2_sign = d2_1 >= 0;

        exp_mult *= 1 + d1 * 0.1;

        if (d2_sign != _last_d2_sign && abs(d1) > 0.05) // 2nd derivative sign change
        {
            exp_mult = (exp_mult + 1) / 2;
        }

        _last_d2_sign = d2_sign;

        _damping_index = (_damping_index + 1) % 2;
        _previous_mults[_damping_index] = exp_mult;
    }

    float oldCcmGain = _ccmGain;

    if (_spCamera)
    {
        float oldShutterSpeed = _spCamera->GetShutterSpeed();
        float oldAnalogueGain = _spCamera->GetAnalogueGain();
        float oldDigitalGain = _spCamera->GetDigitalGain();

        float remaining_gain = exp_mult;// * oldAnalogueGain * oldDigitalGain;

        if(remaining_gain > 1.0)
        {
            _shutterSpeed = CalculateShutterSpeedGain(remaining_gain);
            _analogueGain = CalculateAnalogueGain(remaining_gain);
            _digitalGain = CalculateDigitalGain(remaining_gain);
            _ccmGain = CalculateDigitalCcmGain(remaining_gain);
            _ccmGain = 1.0;
        }
        else
        {
            _ccmGain = CalculateDigitalCcmGain(remaining_gain);
            _ccmGain = 1.0;
            _digitalGain = CalculateDigitalGain(remaining_gain);
            _analogueGain = CalculateAnalogueGain(remaining_gain);
            _shutterSpeed = CalculateShutterSpeedGain(remaining_gain);
        }

        static constexpr float SENSITIVITY_THRESHOLD = 0.01f;

        if (std::max(oldShutterSpeed / _shutterSpeed, _shutterSpeed / oldShutterSpeed) > (1.0f + SENSITIVITY_THRESHOLD))
        {
            _spCamera->SetShutterSpeed(_shutterSpeed);
        }
        else
        {
            _shutterSpeed = oldShutterSpeed;
        }

        if (std::max(oldAnalogueGain / _analogueGain, _analogueGain / oldAnalogueGain) > (1.0f + SENSITIVITY_THRESHOLD))
        {
            _spCamera->SetAnalogueGain(_analogueGain);
        }
        else
        {
            _analogueGain = oldAnalogueGain;
        }

        if (std::max(oldDigitalGain / _digitalGain, _digitalGain / oldDigitalGain) > (1.0f + SENSITIVITY_THRESHOLD))
        {
            _spCamera->SetDigitalGain(_digitalGain);
        }
        else
        {
            _digitalGain = oldDigitalGain;
        }
    }
    else
    {
        _shutterSpeed = 100.0f;
        _analogueGain = 1.0f;
        _digitalGain = 1.0f;
        _ccmGain = 1.0f;
    }

    if (_spCcm)
    {
        if (std::fabs(oldCcmGain - _ccmGain) > 0.1f)
        {
            vvp::ccm::FloatCoefficients coeffs;
            coeffs.A0 = _ccmGain;
            coeffs.B1 = _ccmGain;
            coeffs.C2 = _ccmGain;
            _spCcm->ApplyAEXMatrix(coeffs);
        }

    }
}

void AutoExposureController::SetControlThresholds(bool controlThresholds)
{
    _controlThresholds = controlThresholds;
}

void AutoExposureController::SetDamping(bool damping)
{
    _damping = damping;
}

void AutoExposureController::EnableRoi(bool enable)
{
    _enableRoi = enable;
}

void AutoExposureController::SetRoiRatio(float roiRatio)
{
    _roiRatio = roiRatio;
}

void AutoExposureController::SetRoi(const SwApi::Hs::RegionOfInterest& roi)
{
    _roi = roi;
}

void AutoExposureController::SetTargetMeanBrightness(float meanBrightness)
{
    _meanBrightness = meanBrightness;
}

void AutoExposureController::SetMeanBrightnessStrength(float meanBrightnessStrength)
{
    _meanBrightnessStrength = meanBrightnessStrength;
}

void AutoExposureController::SetDesaturationStrength(float desaturationStrength)
{
    _desaturationStrength = desaturationStrength;
}

void AutoExposureController::SetShadowPreservationStrength(float shadowPreservationStrength)
{
    _shadowPreservationStrength = shadowPreservationStrength;
}

float AutoExposureController::GetShutterSpeed()
{
    return _shutterSpeed;
}

float AutoExposureController::GetAnalogueGain()
{
    return _analogueGain;
}

float AutoExposureController::GetDigitalGain()
{
    float rv = 0.0f;

    if(_spCamera)
    {
        const auto [digital_gain_min, digital_gain_max] = _spCamera->GetDigitalGainRange();

        if (std::fabs(digital_gain_max - digital_gain_min) > std::numeric_limits<float>::epsilon())
            rv = _digitalGain;
        else
            rv = _ccmGain;
    }

    return rv;
}

std::pair<uint32_t, uint32_t> AutoExposureController::GetInputResolution() const
{
    std::pair<uint32_t, uint32_t> rv = {1, 1};

    if(_spHs)
        rv = _spHs->GetResolution();

    return rv;
}

SwApi::Hs::RegionOfInterest AutoExposureController::GetRoiHw() const
{
    SwApi::Hs::RegionOfInterest roi{};

    if(_spHs)
        roi = _spHs->GetRegionOfInterest();

    return roi;
}