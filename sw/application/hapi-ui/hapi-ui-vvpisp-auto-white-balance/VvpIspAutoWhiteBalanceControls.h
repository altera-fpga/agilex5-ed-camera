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
#include "WhiteBalance.h"
#include "Wbs.h"
#include "IROI.h"
#include "IspCommon.h"

class VvpIspAutoWhiteBalanceControls : public IpUiControls
{
    enum class AWBUiModeSelect : uint32_t
    {
        Disabled = 0,
        ChooseTemperatureKelvin = 1,
        ChooseLigthing = 2,
        Automatic = 3,
        CustomPreset = 4
    };

    using RoiCustomControl = UiCustomControlWithValue<RoiSelectorData>;

public:

    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspAutoWhiteBalanceControls(
        const std::string& name,
        const std::shared_ptr<WhiteBalanceController>& spWBController,
        const IROIPtr& spIROI);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void AutoWhiteBalanceUpdateFunc();

    void LoadBlockBlcWbcFunctionPointer(std::function<void(bool)> fpBlockBlcWbcUiCB);
    void SetBypassBLCAndWBCFunctionPointer(std::function<void(bool)> fpSetBypassBlcWbcUiCB);

    bool IsAwbEnabled() const { return (_selectedMode != AWBUiModeSelect::Disabled); }

    void UpdateRoi(bool updateUI);
    void DisableRoiHighlighting();

    void AutoWhiteBalanceReset();

    void SwitchAwbMode(const AWBUiModeSelect& mode);

private:
    void UpdateTint();

    const std::string _name;
    std::shared_ptr<WhiteBalanceController> _spWBController;
    IROIPtr _spIROI;

    std::shared_ptr<UiControlItemEnum> _spWhiteBalanceEnum;
    std::shared_ptr<UiControlItemEnum> _spLightingEnum;
    std::shared_ptr<UiControlItemSlider> _spColTempSlider;
    std::shared_ptr<UiControlItemSlider> _spCurTempSlider;
    std::shared_ptr<UiControlItemSlider> _spTintSlider;
    std::shared_ptr<UiControlItemSlider> _spCcmStrengthSlider;
    std::shared_ptr<UiControlItemButton> _spPresetManualButton;

    std::shared_ptr<UiControlItemButton> _spWbRoiResetButton;
    std::shared_ptr<UiControlItemBoolean> _spWbRoiHighlightBool;
    std::shared_ptr<RoiCustomControl>  _spRoiCustomControl;

    RoiSelectorData _roi;

    bool _highlighting = false;

    bool _shouldCallWBOnSliderChange = false;

    AWBUiModeSelect _selectedMode = AWBUiModeSelect::Disabled;

    std::function<void(bool)> _fpBlockBlcWbcUiCB;
    std::function<void(bool)> _fpSetBypassBlcWbcUiCB;

    const uint32_t _NUM_ROI_HIGHLIGHT_ITER = 20;
    int32_t _wbRoiHighlightCounter;

    bool _controlsInitialised;
};
