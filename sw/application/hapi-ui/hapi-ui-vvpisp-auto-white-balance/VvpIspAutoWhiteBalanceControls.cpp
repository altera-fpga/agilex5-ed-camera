/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspAutoWhiteBalanceControls.h"
#include "UiElements.h"
#include "CoeffGen.h"
#include <string>


VvpIspAutoWhiteBalanceControls::VvpIspAutoWhiteBalanceControls(
    const std::string& name,
    const std::shared_ptr<WhiteBalanceController>& spWBController,
    const IROIPtr& spIROI)
: _name(name),
  _spWBController{spWBController},
  _spIROI{spIROI},
  _wbRoiHighlightCounter{0},
  _controlsInitialised{false}
{
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspAutoWhiteBalanceControls::AddUiElements()
{
    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    auto autoWhiteBalanceResetCB = [this] (uint32_t clientID) -> void
    {
        AutoWhiteBalanceReset();
    };

    spContainer->AddHeaderButtons({{"AutoWhiteBalanceReset", autoWhiteBalanceResetCB, "Reset", "OJL/Images/Reset.png"}});

    auto operationModeDDCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
    {
        _selectedMode = static_cast<AWBUiModeSelect>(selected._userItemData);

        if(_controlsInitialised)
            SwitchAwbMode(_selectedMode);
    };

    std::vector<UiEnumOption> operationModes = {
        { "Disabled",               static_cast<uint32_t>(AWBUiModeSelect::Disabled) },
        { "Choose Temperature",     static_cast<uint32_t>(AWBUiModeSelect::ChooseTemperatureKelvin) },
        { "Choose Lighting",        static_cast<uint32_t>(AWBUiModeSelect::ChooseLigthing) },
        { "Automatic",              static_cast<uint32_t>(AWBUiModeSelect::Automatic) },
        { "Custom Preset (Spot)",   static_cast<uint32_t>(AWBUiModeSelect::CustomPreset) },
    };

    _spWhiteBalanceEnum = spContainer->AddEnumControl(
        "White Balance", operationModes, operationModeDDCB, "awbOperationModeEnum", static_cast<uint32_t>(AWBUiModeSelect::Automatic));

    auto lightingEnumCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
    {
        if (_spColTempSlider)
        {
            _spColTempSlider->UpdateValue((int32_t)selected._userItemData, true, clientID);
            _spColTempSlider->PushValueToUI();
        }
    };

    // White balance lighting preset temperatures
    static constexpr uint32_t AWB_PRESET_SUNSET          = 2700;
    static constexpr uint32_t AWB_PRESET_INCANDESCENT    = 3200;
    static constexpr uint32_t AWB_PRESET_FLUORESCENT     = 4200;
    static constexpr uint32_t AWB_PRESET_SUNLIGHT        = 5200;
    static constexpr uint32_t AWB_PRESET_CLOUDY          = 6000;
    static constexpr uint32_t AWB_PRESET_SHADE           = 7000;
    static constexpr uint32_t AWB_PRESET_BLUESKY         = 10000;

    std::vector<UiEnumOption> wbLightingEnum = {
        { "Sunset",         AWB_PRESET_SUNSET },
        { "Incandescent",   AWB_PRESET_INCANDESCENT },
        { "Fluorescent",    AWB_PRESET_FLUORESCENT },
        { "Sunlight",       AWB_PRESET_SUNLIGHT },
        { "Cloudy",         AWB_PRESET_CLOUDY },
        { "Shade",          AWB_PRESET_SHADE },
        { "Blue Sky",       AWB_PRESET_BLUESKY }
    };

    _spLightingEnum = spContainer->AddEnumControl(
        "Lighting", wbLightingEnum, lightingEnumCB, "wbLightingEnum", 0);

    auto tempChangeCB = [this] (uint32_t clientID, int32_t& value)
    {
        if (_shouldCallWBOnSliderChange)
        {
            _spWBController->SetTemperature(value);
        }
    };

    _spColTempSlider = spContainer->AddSliderControl("Temperature (K)", 2000, 12000, tempChangeCB, "tempScaleSlider", 5700);

    auto TintChangeCB = [this] (uint32_t clientID, float& value)
    {
        _spWBController->SetTint(value);
    };

    _spTintSlider = spContainer->AddSliderControl("Tint Adjustment", 0.0f, -1.0, 1.0, TintChangeCB, 2);
    
    auto StrengthChangeCB = [this] (uint32_t clientID, float& value)
    {
        _spWBController->SetTintStrength(value);
    };

    _spCcmStrengthSlider = spContainer->AddSliderControl("CCM Strength", 1.0f, 0.0f, 2.0f, StrengthChangeCB, 2);

    auto wbCustomPresetCB = [this](uint32_t clientID)
    {
        _spWbRoiHighlightBool->UpdateValue(true, true);
        _wbRoiHighlightCounter = _NUM_ROI_HIGHLIGHT_ITER;
    };

    _spPresetManualButton = spContainer->AddButtonControl("Snapshot", wbCustomPresetCB);

    spContainer->AddSeparator();

    auto wbRoiHighlightCB = [this](uint32_t clientID, bool& value)
    {
        if(_spIROI)
        {
            _highlighting = value;
            _spIROI->SetEnableRoi(_highlighting);
        }

        UpdateRoi(false);
    };

    _spWbRoiHighlightBool = spContainer->AddBoolControl("Highlight Region of Interest", wbRoiHighlightCB, "roiHighlight", false);

    auto wbRoiResetCB = [this](uint32_t clientID)
    {
        _roi._x = 0.0f;
        _roi._y = 0.0f;
        _roi._width = 1.0f;
        _roi._height = 1.0f;
        UpdateRoi(true);
    };

    _spWbRoiResetButton = spContainer->AddButtonControl("Reset Region of Interest", wbRoiResetCB);

    auto roiUpdateCB = [this](uint32_t clientID, RoiSelectorData& value)
    {
        _roi._x = value._x;
        _roi._y = value._y;
        _roi._width = value._width;
        _roi._height = value._height;
        UpdateRoi(false);

        _spWbRoiHighlightBool->UpdateValue(true, true);
        _wbRoiHighlightCounter = _NUM_ROI_HIGHLIGHT_ITER;
    };

    static const std::string wbsRoiDefault{"0.0, 0.0, 1.0, 1.0, false,false,false"};
    auto roiSetting = std::make_shared<SettingValue<std::string>>(GetSettingsSectionName().c_str(), "WbcRoi", wbsRoiDefault);
    _spRoiCustomControl = std::make_shared<RoiCustomControl>("OJRoiControl", _roi, roiUpdateCB, nullptr, roiSetting);
    
    spContainer->Add(_spRoiCustomControl);

    _controlsInitialised = true;
    SwitchAwbMode(_selectedMode);

    if(_spIROI)
        _spIROI->SetEnableRoi(false);

    return {std::move(spContainer)};
}


void VvpIspAutoWhiteBalanceControls::AutoWhiteBalanceUpdateFunc()
{
    switch (_selectedMode)
    {
        default:
        case AWBUiModeSelect::Disabled:
        case AWBUiModeSelect::ChooseTemperatureKelvin:
        case AWBUiModeSelect::ChooseLigthing:
        {
            break;
        }

        case AWBUiModeSelect::CustomPreset:
        {
            if (_wbRoiHighlightCounter <= 0)
            {
                // Spill into automatic only for the ROI highlighting duration
                break;
            }
        }
        case AWBUiModeSelect::Automatic:
        {
            if (_wbRoiHighlightCounter > 0)
            {
                --_wbRoiHighlightCounter;
                if (_wbRoiHighlightCounter == 0)
                {
                    _spWbRoiHighlightBool->UpdateValue(false, true);
                }
            }

            uint16_t sceneTemp = _spWBController->GetCurrentSceneTemperature();

            if (sceneTemp == 0)
            {
                sceneTemp = 5700;
            }

            const auto currentSceneTemp = _spColTempSlider->GetValue<int32_t>();

            if(currentSceneTemp != (int32_t)sceneTemp)
                _spColTempSlider->UpdateValue((int32_t)sceneTemp, false);

            break;
        }
    }

    if (_fpBlockBlcWbcUiCB)
    {
        _fpBlockBlcWbcUiCB(_selectedMode != AWBUiModeSelect::Disabled);
    }
}


void VvpIspAutoWhiteBalanceControls::LoadBlockBlcWbcFunctionPointer(std::function<void(bool)> fpBlockBlcWbcUiCB)
{
    _fpBlockBlcWbcUiCB = std::move(fpBlockBlcWbcUiCB);
}


void VvpIspAutoWhiteBalanceControls::SetBypassBLCAndWBCFunctionPointer(std::function<void(bool)> fpSetBypassBlcWbcUiCB)
{
    _fpSetBypassBlcWbcUiCB = std::move(fpSetBypassBlcWbcUiCB);
}


void VvpIspAutoWhiteBalanceControls::UpdateRoi(bool updateUI)
{
    // Make sure all the flags are correct
    _roi._enabled = true;
    _roi._outside = false;
    _roi._tmo_enabled = true;

    SwApi::WbsRoI wbsRoi;
    const auto [w, h] = _spWBController->GetWbs()->GetResolution();
    wbsRoi.h_start = w * _roi._x;
    wbsRoi.v_start = h * _roi._y;
    wbsRoi.h_end = w * (_roi._x + _roi._width);
    wbsRoi.v_end = h * (_roi._y + _roi._height);
    _spWBController->SetRoi(wbsRoi);

    if (updateUI)
    {
        _spRoiCustomControl->UpdateValue(_roi);
    }

    if (_highlighting)
    {
        _spIROI->SetEnableRoi(true);
        altera_vvp_roi roi = {
            ._x = _roi._x,
            ._y = _roi._y,
            ._width = _roi._width,
            ._height = _roi._height
        };
        _spIROI->SetRegionOfInterest(roi);
    }
}


void VvpIspAutoWhiteBalanceControls::DisableRoiHighlighting()
{
    _spWbRoiHighlightBool->UpdateValue(false, true);
}


void VvpIspAutoWhiteBalanceControls::AutoWhiteBalanceReset()
{
    _spWBController->ApplyBlackLevel();
    _spTintSlider->UpdateValue(0.0f, true);
    _spCcmStrengthSlider->UpdateValue(1.0f, true);

    _roi._x = 0.0f;
    _roi._y = 0.0f;
    _roi._width = 1.0f;
    _roi._height = 1.0f;

    UpdateRoi(true);

    _spWhiteBalanceEnum->Enable(true);
    _spWhiteBalanceEnum->UpdateValue(static_cast<uint32_t>(AWBUiModeSelect::Automatic), true);
}


void VvpIspAutoWhiteBalanceControls::SwitchAwbMode(const AWBUiModeSelect& mode)
{
    switch(mode)
    {
        case AWBUiModeSelect::Disabled:
        {
            _spWBController->SetAWBMode(WhiteBalanceController::AWBMode::Disabled);
            _shouldCallWBOnSliderChange = false;

            _spLightingEnum->Enable(false);
            _spColTempSlider->Enable(false);
            _spTintSlider->Enable(false);
            _spCcmStrengthSlider->Enable(false);
            _spPresetManualButton->Enable(false);

            _spWbRoiResetButton->Enable(false);
            _spWbRoiHighlightBool->UpdateValue(false, true);
            _spWbRoiHighlightBool->Enable(false);
            _roi._enabled = false;
            _spRoiCustomControl->UpdateValue(_roi);
            break;
        }
        case AWBUiModeSelect::ChooseTemperatureKelvin:
        {
            _spWBController->SetAWBMode(WhiteBalanceController::AWBMode::ChooseTemperatureKelvin);
            _shouldCallWBOnSliderChange = true;

            _spLightingEnum->Enable(false);
            _spColTempSlider->Enable(true);
            _spTintSlider->Enable(true);
            _spCcmStrengthSlider->Enable(true);
            _spPresetManualButton->Enable(false);

            _spWbRoiResetButton->Enable(false);
            _spWbRoiHighlightBool->UpdateValue(false, true);
            _spWbRoiHighlightBool->Enable(false);
            _roi._enabled = false;
            _spRoiCustomControl->UpdateValue(_roi);

            _spColTempSlider->UpdateValue(_spColTempSlider->GetValue<int32_t>(), true);
            //_spCurTempSlider->Enable(false); // FIXME - revert after demo
            break;
        }
        case AWBUiModeSelect::ChooseLigthing:
        {
            _spWBController->SetAWBMode(WhiteBalanceController::AWBMode::ChooseLigthing);
            _shouldCallWBOnSliderChange = true;

            _spLightingEnum->Enable(true);
            _spColTempSlider->Enable(false);
            _spTintSlider->Enable(true);
            _spCcmStrengthSlider->Enable(true);
            _spPresetManualButton->Enable(false);

            _spWbRoiResetButton->Enable(false);
            _spWbRoiHighlightBool->UpdateValue(false, true);
            _spWbRoiHighlightBool->Enable(false);
            _roi._enabled = false;
            _spRoiCustomControl->UpdateValue(_roi);

            _spLightingEnum->UpdateValue(_spLightingEnum->GetSelectedIndex(), true);
            //_spCurTempSlider->Enable(false);
            break;
        }
        case AWBUiModeSelect::Automatic:
        {
            _spWBController->SetAWBMode(WhiteBalanceController::AWBMode::Automatic);
            _shouldCallWBOnSliderChange = false;

            _spLightingEnum->Enable(false);
            _spColTempSlider->Enable(false);
            _spTintSlider->Enable(true);
            _spCcmStrengthSlider->Enable(true);
            _spPresetManualButton->Enable(false);

            _spWbRoiResetButton->Enable(true);
            _spWbRoiHighlightBool->Enable(true);
            _roi._enabled = true;
            _spRoiCustomControl->UpdateValue(_roi);
            break;
        }
        case AWBUiModeSelect::CustomPreset:
        {
            _spWBController->SetAWBMode(WhiteBalanceController::AWBMode::CustomPreset);
            _shouldCallWBOnSliderChange = true;

            _spLightingEnum->Enable(false);
            _spColTempSlider->Enable(false);
            _spTintSlider->Enable(true);
            _spCcmStrengthSlider->Enable(true);
            _spPresetManualButton->Enable(true);

            _spWbRoiResetButton->Enable(true);
            _spWbRoiHighlightBool->Enable(true);
            _roi._enabled = true;
            _spRoiCustomControl->UpdateValue(_roi);
            //_spCurTempSlider->Enable(false);
            break;
        }
    }

    if (_selectedMode == AWBUiModeSelect::Disabled)
    {
        if (_fpSetBypassBlcWbcUiCB)
        {
            _fpSetBypassBlcWbcUiCB(true);
        }
    }    
}