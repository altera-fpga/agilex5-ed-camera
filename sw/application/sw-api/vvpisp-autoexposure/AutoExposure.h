/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include "IspCommon.h"
#include "ICamera.h"

#include "HistogramStats.h"
#include "ITmo.h"
#include "Ccm.h"


struct PixelCounts
{
    uint32_t total_count;
    uint32_t sat_count;
    uint32_t auto_count;
    uint32_t under_count;
};

class IMultiChannelAutoExposure
{
public:
    IMultiChannelAutoExposure() {};
    virtual ~IMultiChannelAutoExposure() {};
    virtual void BalanceMean(uint32_t handle, float& mean) = 0;
    virtual void BalanceSaturation(uint32_t handle, float& sat_proportion) = 0;
    virtual void BalanceUnderSaturation(uint32_t handle, float& under_proportion) = 0;
};
using IMultiChannelAutoExposurePtr = std::shared_ptr<IMultiChannelAutoExposure>;

class AutoExposureController
{
public:
    AutoExposureController(const std::shared_ptr<SwApi::HistogramStats>& spHs, 
                            const std::shared_ptr<SwApi::Ccm>& spCcm);
    ~AutoExposureController();
    AutoExposureController(const AutoExposureController& other) = delete;
    AutoExposureController& operator=(const AutoExposureController& other) = delete;
    AutoExposureController(AutoExposureController&& other) = delete;
    AutoExposureController& operator=(AutoExposureController&& other) = delete;

    void RunAutoExposure();

    SwApi::Hs::TableResults GetLumaBins();
    void AdjustThresholds(const SwApi::Hs::TableResults& luma_bins);
    float CalculateExposureMult(const SwApi::Hs::TableResults& luma_bins, const SwApi::Hs::RegionOfInterest& roi);

    void SetThresholds(float underThreshold, float satThreshold);
    // \returns {underThreshold, satThresholds}
    std::pair<float, float> GetThresholds();
    void SetControlThresholds(bool controlThresholds);
    void SetDamping(bool damping);
    void SetManualExposure(bool enable);
    void EnableRoi(bool enable);
    void SetRoiRatio(float roiRatio);
    void SetRoi(const SwApi::Hs::RegionOfInterest& roi);
    void SetTargetMeanBrightness(float meanBrightness);
    void SetMeanBrightnessStrength(float meanBrightnessStrength);
    void SetDesaturationStrength(float desaturationStrength);
    void SetShadowPreservationStrength(float shadowPreservationStrength);

    float GetShutterSpeed();
    float GetAnalogueGain();
    float GetDigitalGain();

    void SetCamera(ICameraPtr spCamera);

    void SetMultiChannelAutoExposure(IMultiChannelAutoExposurePtr& spMultiChannelAutoExposure, uint32_t handle);

    std::pair<uint32_t, uint32_t> GetInputResolution() const;
    SwApi::Hs::RegionOfInterest GetRoiHw() const;

private:
    float CalculateExposureGain(float& gain);
    float CalculateShutterSpeedGain(float& gain);
    float CalculateAnalogueGain(float& gain);
    float CalculateDigitalGain(float& gain);
    float CalculateDigitalCcmGain(float& gain);
    void CheckCameraSettings();

    std::shared_ptr<SwApi::HistogramStats>  _spHs;
    std::shared_ptr<SwApi::Ccm>             _spCcm;
    ICameraPtr                              _spCamera;
    IMultiChannelAutoExposurePtr            _spMultiChannelAutoExposure;
    uint32_t                                _multiChannelAutoExposureHandle;

    static constexpr std::size_t FRAMES_AVERAGED = 1;

    std::shared_ptr<SwApi::Hs::TableResults> _tableValues[FRAMES_AVERAGED];
    std::size_t _totalFrames = 0;

    bool _damping = false;
    bool _controlThresholds = false;
    bool _enableRoi = false;
    float _roiRatio = 0.5;

    float _underThreshold = 0.05;
    float _meanBrightness = 0.25;
    float _satThreshold = 0.9;

    float _meanBrightnessStrength = 1.0;
    float _desaturationStrength = 0.8;
    float _shadowPreservationStrength = 0.2;

    const float _mean_brightness_convergence = 1.0;
    const float _desaturation_convergence = 1.0;
    const float _shadow_preservation_convergence = 1.0;

    const float _sat_tolerance = 0.05;
    const float _under_tolerance = 0.1;

    const float _min_sat_mult = 0.1;
    const float _max_under_mult = 10;


    float _shutterSpeed = 100.0;
    float _analogueGain = 1.0;
    float _digitalGain = 1.0;
    float _ccmGain = 1.0;

    SwApi::Hs::RegionOfInterest _roi;
    SwApi::Hs::RegionOfInterest _previous_roi;

    std::size_t _frame_index;
    float _previous_mults[3];
    int _damping_index;
    bool _last_d2_sign;

    SwApi::HistogramStats::HistogramSubscriptionId _histogramSubscriptionId = 0;
    std::mutex _histogramDataMutex;
};
