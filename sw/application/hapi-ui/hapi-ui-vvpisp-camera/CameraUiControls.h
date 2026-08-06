/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <string>
#include <memory>
#include "IpUiControls.h"
#include "ICamera.h"
#include "HapiVvpExposureFusion.h"


class CameraUiControls : public IpUiControls
{
public:
    CameraUiControls(const ICameraPtr camera, const std::string& name, Hapi::VvpExposureFusionPtr exposureFusion, const bool debugControls = false);
    virtual ~CameraUiControls();

    virtual std::string GetSettingsSectionName() override;

    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    void SetAnalogueGainCb(std::function<void(const float)> analogueGainCb){
        _analogueGainCb = analogueGainCb;
    }

    void SetShutterSpeed(const float v);
    float GetShutterSpeed() const;
    void SetAnalogueGain(const float v);
    float GetAnalogueGain() const;
    void SetDigitalGain(const float v);
    float GetDigitalGain() const;

    void SetActive(bool isActive);

    void EnableHdr(bool v);

    void UpdateHdrBlackLevel(const std::array<uint32_t, 4>& blc);
    void UpdateHdrThreshold(const uint32_t blc, const uint32_t ratio);

private:
    static constexpr float HDR_MAX_FRAME_RATE = 25.0f;

    virtual std::string GetModelName();
    bool SetExposureFusionBlackLevel(const uint32_t v);
    bool SetExposureFusionRatio(const uint32_t v);
    bool SetExposureFusionThreshold(const uint32_t v);

    void UpdateFrameRateSlider(float frameRate);
    void UpdateShutterSpeedSlider();

    static constexpr uint32_t _HDR_EXPOSURE_RATIO_DEFAULT = 10802;
    static constexpr uint32_t _HDR_GAIN_12dB_DEFAULT = 2;

    ICameraPtr _camera;
    std::string _name;

    Hapi::VvpExposureFusionPtr _exposureFusion;

    std::shared_ptr<UiControlItemSlider> _frameRateControl;
    std::shared_ptr<UiControlItemSlider> _shutterControl;
    std::shared_ptr<UiControlItemSlider> _analogueGainControl;
    std::shared_ptr<UiControlItemSlider> _digitalGainControl;

    std::shared_ptr<UiControlItemSlider> _focusControl;

    std::function<void(const float)> _analogueGainCb;

    // Debug controls
    std::shared_ptr<UiControlItemUInteger> _spAddressBox;
    std::shared_ptr<UiControlItemUInteger> _spDataBox;

    bool _debugControls;

    // Exposure fusion controls
    std::shared_ptr<UiControlItemEnum> _spModeEnum;
    std::shared_ptr<UiControlItemEnum> _spGainEnum;
    std::shared_ptr<UiControlItemUInteger> _spBlackLevel;
    std::shared_ptr<UiControlItemUInteger> _spRatio;
    std::shared_ptr<UiControlItemUInteger> _spThreshold;

    std::shared_ptr<UiControlContainer> _spContainer;

    float _frameRate;
    float _frameRateHdr;
};

using CameraUiControlsPtr = std::shared_ptr<CameraUiControls>;