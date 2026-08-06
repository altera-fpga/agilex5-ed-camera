/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <array>

#include "IspCommon.h"
#include "ICamera.h"

#include "CalibrationProfile.h"
#include "Bls.h"
#include "Blc.h"
#include "Anr.h"
#include "Ccm.h"
#include "Wbs.h"
#include "Wbc.h"
#include "Vc.h"
#include "SwitchRouter.h"
#include "SwUtilsThread.h"
#include "WbsUtils.h"
#include "VcMeshUtils.h"


class IMultiChannelWhiteBalance
{
public:
    IMultiChannelWhiteBalance() {};
    virtual ~IMultiChannelWhiteBalance() {};
    virtual void BalanceTemp(uint32_t handle, uint32_t& temp) = 0;
};
using IMultiChannelWhiteBalancePtr = std::shared_ptr<IMultiChannelWhiteBalance>;

class WhiteBalanceController : public SwUtils::Thread
{
public:
    enum class AWBMode : uint32_t
    {
        Disabled = 0,
        ChooseTemperatureKelvin = 1,
        ChooseLigthing = 2,
        Automatic = 3,
        CustomPreset = 4
    };

    WhiteBalanceController(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                           const std::shared_ptr<SwApi::Blc>& spBlackLevelCorrect,
                           const std::shared_ptr<SwApi::Anr>& spAdaptiveNoiseReduction,
                           const std::shared_ptr<SwApi::Vc>& spVignetteCorrection,
                           const std::shared_ptr<SwApi::Wbs>& spWhiteBalanceStats,
                           const std::shared_ptr<SwitchRouter>& spWbsSwitchRouter,
                           const std::shared_ptr<SwApi::Wbc>& spWhiteBalanceCorrection,
                           const std::shared_ptr<SwApi::Ccm>& spColourCorrectionMatrix,
                           const char* const name = "WhiteBlncCtrl");

    ~WhiteBalanceController();
    WhiteBalanceController(const WhiteBalanceController& other) = delete;
    WhiteBalanceController& operator=(const WhiteBalanceController& other) = delete;
    WhiteBalanceController(WhiteBalanceController&& other) = delete;
    WhiteBalanceController& operator=(WhiteBalanceController&& other) = delete;


    void SetAWBMode(AWBMode awbMode);
    void SetMultiChannelWhiteBalance(const IMultiChannelWhiteBalancePtr& spMultiChannelWhiteBalance, uint32_t handle);

    TCfaPhase GetWbcCfaPhase();

    void SetGain(float gain);
    void SetRoi(SwApi::WbsRoI roi);

    void ApplyBlackLevel();

    void DeactivateCcmModifiers();

    void ApplyWhiteBalance();
    void ApplyWhiteBalance(uint16_t currentTemp, uint16_t targetTemp, TCfaPhase cfaPhase);

    // TODO - this isn't white balance. A re-brand is in order
    void ApplyVCMeshes();

    uint16_t GetCurrentSceneTemperature();
    std::vector<float> CalculateRGBScalarsToMatchTempToTarget(uint16_t currentTemp, uint16_t targetTemp);
    std::vector<float> CalculateRGBScalarsToCorrectSceneWhiteLevel(SwApi::NormalisedWbsRegion wbsRatio, uint16_t targetTemp);
    std::vector<uint32_t> CfaAlignedRGBScale(float red, float green, float blue, TCfaPhase cfaPhase);
    SwApi::WbsResults ReadWBSAndGenerateCoeffsForTemp(uint16_t temp, TCfaPhase cfaPhase, float* wbcScalars, uint32_t* wbcCoeffs, uint32_t* bestX, uint32_t* bestY);
    void ResetWBSToDefault();
    void ReadWBSAndUpdateProfile(TCfaPhase cfaPhase, uint16_t temp, uint32_t *params);
    //void ReadWBSAndUpdateProfileBaseline(TCfaPhase cfaPhase, float gain, uint32_t *params);

    std::shared_ptr<SwApi::Wbs> GetWbs() { return _spWhiteBalanceStats; };
    std::shared_ptr<SwApi::Wbc> GetWbc() { return _spWhiteBalanceCorrection; };
    std::shared_ptr<SwApi::Ccm> GetCcm() { return _spColourCorrectionMatrix; };

    void SetTint(float tint);
    void SetTintStrength(float tintStrength);
    void SetTemperature(uint16_t temp);

    void SetBlackLevelCallback(std::function<void(std::array<uint32_t, 4>)> blackLevelCb)
    {
        _blackLevelCb = blackLevelCb;
    }

    virtual void RunThread() override;

protected:
    uint16_t MesureCurrentSceneTemperature(bool applyUpdateToWbs);
    void UpdateTint();
    void AutoWhiteBalanceUpdateFunc();


private:

    std::shared_ptr<SensorCalibrationProfile> _spProfile;

    AWBMode _awbMode;
    float _tint;
    float _tintStrength;

    // WhiteBalanceSystem autoExpSystem,
    std::shared_ptr<SwApi::Blc> _spBlackLevelCorrect;
    std::shared_ptr<SwApi::Anr> _spAdaptiveNoiseReduction;
    std::shared_ptr<SwApi::Vc> _spVignetteCorrection;
    std::shared_ptr<SwApi::Wbs> _spWhiteBalanceStats;
    std::shared_ptr<SwitchRouter> _spWbsSwitchRouter;
    std::shared_ptr<SwApi::Wbc> _spWhiteBalanceCorrection;
    std::shared_ptr<SwApi::Ccm> _spColourCorrectionMatrix;

    IMultiChannelWhiteBalancePtr _spMultiChannelWhiteBalance;
    uint32_t _multiChannelWhiteBalanceHandle;

    std::vector<uint16_t> _measuredTempSamples;
    uint8_t _tempSampleCircularPointer = 0;
    uint8_t _noMeasuredTempSamplesToAverage = 10;

    float _gain;
    SwApi::WbsRoI _roi;

    std::function<void(std::array<uint32_t, 4>)> _blackLevelCb;

    uint16_t _sceneTemp = 5700;
    uint16_t _currentTemp = 5700;
    uint16_t _targetTemp = 5700;
    TCfaPhase _cfaPhase = TCfaPhase::RGGB;

    bool _print_info = false;

};
