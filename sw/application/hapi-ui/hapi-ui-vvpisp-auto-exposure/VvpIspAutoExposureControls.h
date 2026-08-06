/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "IpUiControls.h"
#include "IROI.h"
#include "IspCommon.h"
#include "AutoExposure.h"

class VvpIspAutoExposureControls : public IpUiControls
{
    using RoiCustomControl = UiCustomControlWithValue<RoiSelectorData>;
public:
    VvpIspAutoExposureControls(const std::string& name, const std::shared_ptr<AutoExposureController>& spAutoExposure,
                 const IROIPtr& spIROI, bool powerUser, bool debugUI);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void SetAutoUpdateToggleFunc(std::function<void(bool)> fpAutoExpAutoControlToggleCb);
    void AeTask();

    void Enable(bool enable);
    bool AeEnabled() const { return _aeEnabled; }

    void DisableRoiHighlighting();

private:
    void UpdateRoi(bool updateUI);
    void AutoExposureReset();
    void UpdateGain();
    void OnRoiControlsChange();

    const std::string _name;

    std::shared_ptr<AutoExposureController> _spAutoExposure;
    IROIPtr _spIROI;

    std::shared_ptr<UiControlContainer> _spControlContainer;

    std::shared_ptr<UiControlContainer> _spRoiPanel;
    std::shared_ptr<UiControlContainer> _spRoiEditorPanel;
    std::shared_ptr<UiControlContainer> _spRoiXYPanel;
    std::shared_ptr<UiControlContainer> _spRoiWHPanel;
    std::shared_ptr<UiControlContainer> _spRoiHighlightPanel;

    std::shared_ptr<UiControlItemBoolean> _spAeEnable;
    std::shared_ptr<UiControlItemLabel> _spShutterSpeedLabel;
    std::shared_ptr<UiControlItemLabel> _spAnalogueGainLabel;
    std::shared_ptr<UiControlItemLabel> _spDigitalGainLabel;
    std::shared_ptr<UiControlItemButton> _spUpdateExp;
    std::shared_ptr<UiControlItemBoolean> _spManualExposure;

    std::shared_ptr<UiControlItemSlider> _spUnderThreshold;
    std::shared_ptr<UiControlItemSlider> _spMeanBrightness;
    std::shared_ptr<UiControlItemSlider> _spMeanBrightnessStrength;
    std::shared_ptr<UiControlItemSlider> _spDesaturationStrength;
    std::shared_ptr<UiControlItemSlider> _spShadowPreservationStrength;
    std::shared_ptr<UiControlItemSlider> _spSatThreshold;
    std::shared_ptr<UiControlItemBoolean> _spControlThresholds;
    std::shared_ptr<UiControlItemBoolean> _spDamping;

    const uint32_t _NUM_ROI_HIGHLIGHT_ITER = 10;
    int32_t _aeRoiHighlightCounter;

    std::shared_ptr<UiControlItemBoolean> _spEnableRoi;
    std::shared_ptr<UiControlItemSlider> _spRoiRatio;
    std::shared_ptr<UiControlItemBoolean> _spAeRoiHighlightBool;
    std::shared_ptr<UiControlItemButton> _spAeRoiResetButton;
    std::shared_ptr<RoiCustomControl> _spRoiCustomControl;

    // Algorithm constants
    float _under_threshold = 0.05;
    float _mean_brightness = 0.4;
    float _sat_threshold = 0.9;

    float _mean_brightness_strength = 1.0;
    float _desaturation_strength = 0.2;
    float _shadow_preservation_strength = 0.2;

    RoiSelectorData _roi;

    bool _highlighting = false;

    std::function<void(bool)> _fpAutoExpAutoControlToggleCb;

    bool _powerUser;
    bool _debugUI;
    bool _aeEnabled;
};
