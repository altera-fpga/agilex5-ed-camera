/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "VvpIspAutoExposureControls.h"
#include <cmath>


VvpIspAutoExposureControls::VvpIspAutoExposureControls(const std::string& name, 
            const std::shared_ptr<AutoExposureController>& spAutoExposure,
            const IROIPtr& spIROI, bool powerUser, bool debugUI)
    : _name(name),
    _spAutoExposure(spAutoExposure),
    _spIROI(spIROI),
    _aeRoiHighlightCounter(0),
    _powerUser(powerUser),
    _debugUI(debugUI),
    _aeEnabled(true)
{
}


void VvpIspAutoExposureControls::AeTask()
{
    if (_aeRoiHighlightCounter > 0)
    {
        --_aeRoiHighlightCounter;
        if (_aeRoiHighlightCounter == 0)
        {
            _spAeRoiHighlightBool->UpdateValue(false, true);
        }
    }

    UpdateGain();

    // Also if ROI is not actively being changed in this tile
    // check if it's changed elsewhere (Pipeline sats tab)
    // and update UI
    if (_aeRoiHighlightCounter == 0)
    {
        const auto [width, height] = _spAutoExposure-> GetInputResolution();

        if(width && height)
        {
            auto to_norm_roi = [](const uint32_t v, const uint32_t dim)->float {
                return static_cast<float>(v) / static_cast<float>(dim);
            };

            const auto roiHw = _spAutoExposure->GetRoiHw();

            const float x = to_norm_roi(roiHw.h_start, width - 1);
            const float y = to_norm_roi(roiHw.v_start, height - 1);
            const float w = to_norm_roi(roiHw.h_end - roiHw.h_start, width - 1);
            const float h = to_norm_roi(roiHw.v_end - roiHw.v_start, height - 1);

            auto float_diff = [](const float v1, const float v2)->bool {
                return std::fabs(v1 - v2) > 0.00001f;
            };

            if(float_diff(x, _roi._x) || (float_diff(y, _roi._y) ||
            float_diff(w, _roi._width) || float_diff(h, _roi._height)))
            {
                _roi._x = x;
                _roi._y = y;
                _roi._width = w;
                _roi._height = h;

                _spRoiCustomControl->UpdateValue(_roi);
            }
        }
    }
}


void VvpIspAutoExposureControls::SetAutoUpdateToggleFunc(std::function<void(bool)> fpAutoExpAutoControlToggleCb)
{
    _fpAutoExpAutoControlToggleCb = std::move(fpAutoExpAutoControlToggleCb);
}


void VvpIspAutoExposureControls::Enable(bool enable)
{
    if(_spControlContainer)
        _spControlContainer->SetEnabled(enable);
}

void VvpIspAutoExposureControls::UpdateGain()
{
    _spAutoExposure->RunAutoExposure();

    if (_debugUI || _powerUser)
    {
        // Update Labels
        std::stringstream shutter_speed_label;
        shutter_speed_label << _spAutoExposure->GetShutterSpeed();
        std::stringstream analogue_gain_label;
        analogue_gain_label << _spAutoExposure->GetAnalogueGain();
        std::stringstream digital_gain_label;
        digital_gain_label << _spAutoExposure->GetDigitalGain();
        _spShutterSpeedLabel->UpdateValue(shutter_speed_label.str());
        _spAnalogueGainLabel->UpdateValue(analogue_gain_label.str());
        _spDigitalGainLabel->UpdateValue(digital_gain_label.str());

        // Only update UI if the actual values changed
        static float underThreshold = 0.0f;
        static float satThreshold = 0.0f;

        auto values_nearly_same = [](const float v1, const float v2)->bool {
            static constexpr float EPSILON = 0.0000001f;
            return std::fabs(v1 - v2) < EPSILON;
        };

        auto thresholds = _spAutoExposure->GetThresholds();

        if(!values_nearly_same(underThreshold, thresholds.first))
        {
            underThreshold = thresholds.first;
            _spUnderThreshold->UpdateValue(underThreshold);
        }

        if(!values_nearly_same(satThreshold, thresholds.second))
        {
            satThreshold = thresholds.second;
            _spSatThreshold->UpdateValue(satThreshold);
        }
    }
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspAutoExposureControls::AddUiElements()
{
    if (!(_spAutoExposure))
        return {};

    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    auto autoExposureResetCB = [this] (uint32_t clientID) -> void
    {
        AutoExposureReset();
    };

    spContainer->AddHeaderButtons({{"AutoWhiteBalanceReset", autoExposureResetCB, "Reset", "OJL/Images/Reset.png"}});

    auto autoUpdateCB = [this](uint32_t clientID, bool value)
    {
        _aeEnabled = value;

        if (_fpAutoExpAutoControlToggleCb)
            _fpAutoExpAutoControlToggleCb(value);
    };

    _spAeEnable = spContainer->AddBoolControl("Enable", autoUpdateCB, "exposureAutoUpdate", true);

    if (_debugUI || _powerUser)
    {
        _spShutterSpeedLabel = spContainer->AddLabelControl("Shutter Speed:", "");
        _spAnalogueGainLabel = spContainer->AddLabelControl("Analog Gain:", "");
        _spDigitalGainLabel = spContainer->AddLabelControl("Digital Gain:", "");
    }

    auto meanBrightnessCB = [this] (uint32_t clientID, float& value) -> void
    {
        _mean_brightness = value;
        _spAutoExposure->SetTargetMeanBrightness(value);
    };

    _spMeanBrightness = spContainer->AddSliderControl("Target Mean Brightness", 0.1, 0.8, meanBrightnessCB, "meanBrightness", 0.3, 1);

    if (_debugUI || _powerUser)
    {
        auto meanBrightnessStrengthCB = [this] (uint32_t clientID, float& value) -> void
        {
            _mean_brightness_strength = value;
            _spAutoExposure->SetMeanBrightnessStrength(value);
        };
        _spMeanBrightnessStrength = spContainer->AddSliderControl("Mean Brightness Strength", 0.0, 1.0, meanBrightnessStrengthCB, "meanBrightnessStrength", 1.0, 1);
    }

    auto desaturationStrengthCB = [this] (uint32_t clientID, float& value) -> void
    {
        _desaturation_strength = value;
        _spAutoExposure->SetDesaturationStrength(value);
        _spAutoExposure->SetShadowPreservationStrength(1.0f - value);
        if (_spShadowPreservationStrength)
        {
            _spShadowPreservationStrength->UpdateValue(1.0f - value);
        }
    };

    _spDesaturationStrength = spContainer->AddSliderControl("Desaturation Strength", 0.0, 1.0, desaturationStrengthCB, "desaturationStrength", 0.8, 1);

    auto shadowPreservationStrengthCB = [this] (uint32_t clientID, float& value) -> void
    {
        _shadow_preservation_strength = value;
        _spAutoExposure->SetShadowPreservationStrength(value);
        _spAutoExposure->SetDesaturationStrength(1.0f - value);
        if (_spDesaturationStrength)
        {
            _spDesaturationStrength->UpdateValue(1.0f - value);
        }
    };
    
    _spShadowPreservationStrength = spContainer->AddSliderControl("Shadow Preservation Strength", 0.0, 1.0, shadowPreservationStrengthCB, "saturationAvoidanceStrength", 0.2, 1);

    spContainer->AddSeparator();

    auto enableRoiCB = [this](uint32_t clientID, bool value)
    {
        _spAutoExposure->EnableRoi(value);
    };

    _spEnableRoi = spContainer->AddBoolControl("Use Region of Interest", enableRoiCB, "enableRoi", false);

    auto roiRatioCB = [this] (uint32_t clientID, float& value) -> void
    {
        _spAutoExposure->SetRoiRatio(value);
    };

    _spRoiRatio = spContainer->AddSliderControl("Region of Interest Contribution", 0.0, 1.0, roiRatioCB, "roiRatio", 0.8, 1);

    auto wbRoiHighlightCB = [this](uint32_t clientID, bool& value)
    {
        if(_spIROI)
        {
            _highlighting = value;
            _spIROI->SetEnableRoi(_highlighting);
        }

        UpdateRoi(false);
    };

    _spAeRoiHighlightBool = spContainer->AddBoolControl("Highlight Region of Interest", wbRoiHighlightCB, "roiHighlight", false);

    auto wbRoiResetCB = [this](uint32_t clientID)
    {
        _roi._x = 0.25f;
        _roi._y = 0.25f;
        _roi._width = 0.5f;
        _roi._height = 0.5f;
        UpdateRoi(true);
    };

    _spAeRoiResetButton = spContainer->AddButtonControl("Reset Region of Interest", wbRoiResetCB);

    auto roiUpdateCB = [this](uint32_t clientID, RoiSelectorData& value)
    {
        _roi._x = value._x;
        _roi._y = value._y;
        _roi._width = value._width;
        _roi._height = value._height;
        UpdateRoi(false);

        if(_aeEnabled)
        {
            _spAeRoiHighlightBool->UpdateValue(true, true);
            _aeRoiHighlightCounter = _NUM_ROI_HIGHLIGHT_ITER;
        }
    };

    // AE ROI defaults to the middle of the screen as
    // Edges and borders are of little interest, also avoids dark areas
    // of the fisheye lenses
    static const std::string aeRoiDefault{"0.25, 0.25, 0.5, 0.5, false,false,false"};
    auto roiSetting = std::make_shared<SettingValue<std::string>>(GetSettingsSectionName().c_str(), "AeRoi", aeRoiDefault);    
    _spRoiCustomControl = std::make_shared<RoiCustomControl>("OJRoiControl", _roi, roiUpdateCB, nullptr, roiSetting);
    spContainer->Add(_spRoiCustomControl);

    //////////////////////////////////////////////////////
    // DEBUG/DEVELOPMENT PARAMETERS
    //////////////////////////////////////////////////////
    if (_debugUI || _powerUser)
    {
        spContainer->AddSeparator();

        auto underCB = [this] (uint32_t clientID, float& value) -> void
        {
            _under_threshold = value;
            _spAutoExposure->SetThresholds(value, _sat_threshold);
        };
        _spUnderThreshold = spContainer->AddSliderControl("Under Threshold", 0.01, 0.5, underCB, "underThreshold", 0.05, 2);

        auto satCB = [this] (uint32_t clientID, float& value) -> void
        {
            _sat_threshold = value;
            _spAutoExposure->SetThresholds(_under_threshold, value);
        };
        _spSatThreshold = spContainer->AddSliderControl("Sat Threshold", 0.5, 0.99, satCB, "satThreshold", 0.9, 2);

        auto controlThresholdsCB = [this] (uint32_t clientID, bool& value) -> void
        {
            _spAutoExposure->SetControlThresholds(value);

            if (value)
            {
                _spUnderThreshold->Enable(false);
                _spSatThreshold->Enable(false);
            }
            else
            {
                _spUnderThreshold->Enable(true);
                _spSatThreshold->Enable(true);
            }
        };
        _spControlThresholds = spContainer->AddBoolControl("Control Thresholds", false, controlThresholdsCB);

        auto dampingCB = [this] (uint32_t clientID, bool& value) -> void
        {
            _spAutoExposure->SetDamping(value);
        };
        _spDamping = spContainer->AddBoolControl("Damping", dampingCB, "damping", false);
    }

    _spControlContainer = spContainer;

    _spRoiCustomControl->UpdateValue(_roi);

    if(_spIROI)
        _spIROI->SetEnableRoi(false);

    return {std::move(spContainer)};
}


void VvpIspAutoExposureControls::UpdateRoi(bool updateUI)
{
    // Make sure all the flags are correct
    _roi._enabled = true;
    _roi._outside = false;
    _roi._tmo_enabled = true;

    if (updateUI)
    {
        _spRoiCustomControl->UpdateValue(_roi);
    }

    const auto [w, h] = _spAutoExposure->GetInputResolution();

    auto to_pixel_roi = [](const float v, const uint32_t dim)->uint16_t {
        return static_cast<uint16_t>(v * static_cast<float>(dim) + 0.5f);
    };

    SwApi::Hs::RegionOfInterest hsRoi =
    {
        .h_start = to_pixel_roi(_roi._x, w-1),
        .v_start = to_pixel_roi(_roi._y, h-1),
        .h_end = to_pixel_roi((_roi._x + _roi._width), w-1),
        .v_end = to_pixel_roi((_roi._y + _roi._height), h-1)
    };

    _spAutoExposure->SetRoi(hsRoi);

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

void VvpIspAutoExposureControls::DisableRoiHighlighting()
{
    _spAeRoiHighlightBool->UpdateValue(false, true);
}

void VvpIspAutoExposureControls::AutoExposureReset()
{
    _spAeEnable->UpdateValue(true, true);
    _spMeanBrightness->UpdateValue(0.3f, true);

    if (_spMeanBrightnessStrength)
        _spMeanBrightnessStrength->UpdateValue(1.0f, true);
    
    _spDesaturationStrength->UpdateValue(0.8f, true);
    _spShadowPreservationStrength->UpdateValue(0.2f, true);
    _spEnableRoi->UpdateValue(true, true);
    _spRoiRatio->UpdateValue(0.8f, true);
    _spAeRoiHighlightBool->UpdateValue(false, true);

    _roi._x = 0.25f;
    _roi._y = 0.25f;
    _roi._width = 0.5f;
    _roi._height = 0.5f;

    UpdateRoi(true);
}
