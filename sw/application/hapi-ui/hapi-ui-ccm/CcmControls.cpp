/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CcmControls.h"
#include "UiElements.h"

using namespace SwApi;

CcmControls::CcmControls(const std::string& name,
                         const std::shared_ptr<SwApi::Ccm>& spCcm,
                         bool enableDebugUi)
    : _name(name)
    , _spCcm(spCcm)
    , _enableDebugUi(enableDebugUi)
{
    _hsvMatrix = vvp::ccm::GenerateIdentityMatrix();
    _temperatureMatrix = vvp::ccm::GenerateIdentityMatrix();
    _strengthMatrix = vvp::ccm::GenerateIdentityMatrix();
    _contrastMatrix = vvp::ccm::GenerateIdentityMatrix();
    ResetParamsAndRecalculate();
}

void CcmControls::GetColourStrengths(float& r, float& g, float& b)
{
    r = _rStrength;
    g = _gStrength;
    b = _bStrength;
}

void CcmControls::SetColourStrengths(float r, float g, float b)
{
    _spRStrength->UpdateValue(r, true);
    _spGStrength->UpdateValue(g, true);
    _spBStrength->UpdateValue(b, true);
}

void CcmControls::RecalculateResultantMatrix()
{
    bool cascadeChanges = false;

    if (_updateHSV)
    {
        _hsvMatrix = vvp::ccm::GenerateHSVMatrix(_hue, _sat, _val);

        cascadeChanges = true;
    }
    if (cascadeChanges || _updateTemperature)
    {
        _temperatureMatrix = _hsvMatrix;

        if (_shouldAdjustTemperature)
        {
            vvp::ccm::ModifyMatrixByColorTemperature(_temperatureMatrix, _temperature, _tint);
        }
        
        cascadeChanges = true;
    }
    if (cascadeChanges || _updateStrength)
    {
        _strengthMatrix = _temperatureMatrix;
        
        vvp::ccm::ScaleChannelStrength(_strengthMatrix, _rStrength, _gStrength, _bStrength);
        
        cascadeChanges = true;
    }
    if (cascadeChanges || _updateContrastAndFloor)
    {
        _contrastMatrix = _strengthMatrix;

        vvp::ccm::IncreaseChannelContrast(_contrastMatrix, _contrast, _contrast, _contrast);
        vvp::ccm::RaiseChannelFloor(_contrastMatrix, _floor, _floor, _floor);

        cascadeChanges = true;
    }

    if(_spCcm)
        _spCcm->ApplyToyMatrix(_contrastMatrix);

    _updateHSV = false;
    _updateTemperature = false;
    _updateStrength = false;
    _updateContrastAndFloor = false;
}

void CcmControls::ResetParamsAndRecalculate()
{
    // These settings are calibrated for the terasic cameras
    _hue = 0;
    _sat = 1.0;
    _val = 1.0;
    _temperature = 6600;
    _tint = 0.0;
    _rStrength = 1.0;
    _gStrength = 1.0;
    _bStrength = 1.0;
    _contrast = 0.0;
    _floor = 0;
    _summand_scale = 10;

    _updateHSV = true;
    _updateTemperature = true;
    _updateStrength = true;
    _updateContrastAndFloor = true;
    _format = false;
    _shouldAdjustTemperature = false;

    // If the UI is present, we can reset it too
    if (_spHueSlider)
    {
        _spHueSlider->UpdateValue(_hue);
        _spSatSlider->UpdateValue(_sat);
        _spValSlider->UpdateValue(_val);
        _spEnableTempBool->UpdateValue(_shouldAdjustTemperature);
        _spTempSlider->UpdateValue(0.0f);
        _spTintSlider->UpdateValue(_tint);
        _spRStrength->UpdateValue(_rStrength);
        _spGStrength->UpdateValue(_gStrength);
        _spBStrength->UpdateValue(_bStrength);
        _spContrastSlider->UpdateValue(_contrast);
        _spImageFloorSlider->UpdateValue(_floor);
        if (_spSummScaleSlider)
        {
            _spSummScaleSlider->UpdateValue(_summand_scale);
        }
    }

    RecalculateResultantMatrix();
}

std::vector<std::shared_ptr<UiControlContainer>> CcmControls::AddUiElements() 
{

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());
    if (not _spCcm) 
    {
        spContainer->AddLabelControl("CCM CSC is not present");
        return {std::move(spContainer)};
    }

    auto hueChangeCB = [this] (uint32_t clientID, float& value)
    {
        _hue = value;
        _updateHSV = true;

        RecalculateResultantMatrix();
    };
    auto satChangeCB = [this] (uint32_t clientID, float& value)
    {
        _sat = value;
        _updateHSV = true;

        RecalculateResultantMatrix();
    };
    auto valChangeCB = [this] (uint32_t clientID, float& value)
    {
        _val = value;
        _updateHSV = true;

        RecalculateResultantMatrix();
    };
    _spHueSlider = spContainer->AddSliderControl("Hue", _hue, 0.0, 360.0, hueChangeCB, 2);
    _spSatSlider = spContainer->AddSliderControl("Saturation", _sat, 0.0, 2.0, satChangeCB, 2);
    _spValSlider = spContainer->AddSliderControl("Value", _val, 0.0, 1.0, valChangeCB, 2);

    auto EnableTempAdjustmentCB = [this](uint32_t clientID, bool &val) -> void
    {
        _shouldAdjustTemperature = val;
        _updateTemperature = true;

        RecalculateResultantMatrix();
    };

    auto TempChangeCB = [this] (uint32_t clientID, float& value)
    {
        const int minTemp = 2000;
        const int medTemp = 6600;
        const int maxTemp = 20000;
        if (value < 0.0)
        {
            // Value is negative, so this becomes a minus without needing an abs
            _temperature = (float)medTemp + (value * (float)(medTemp - minTemp));
        }
        else
        {
            _temperature = (float)medTemp + (value * (float)(maxTemp - medTemp));
        }
        
        _updateTemperature = true;

        RecalculateResultantMatrix();
    };
    auto TintChangeCB = [this] (uint32_t clientID, float& value)
    {
        _tint = value;
        _updateTemperature = true;

        RecalculateResultantMatrix();
    };
    _spEnableTempBool = spContainer->AddBoolControl("Enable Color Temp Post Process", EnableTempAdjustmentCB, "tempEnableToggle", _shouldAdjustTemperature);
    _spTempSlider = spContainer->AddSliderControl("Post Process Temp Modifier", 0.0, -1.0, 1.0, TempChangeCB, 2);
    _spTintSlider = spContainer->AddSliderControl("Post Process Tint Modifier", _tint, -1.0, 1.0, TintChangeCB, 2);

    auto RChangeCB = [this] (uint32_t clientID, float& value)
    {
        _rStrength = value;
        _updateStrength = true;

        RecalculateResultantMatrix();
    };
    auto GChangeCB = [this] (uint32_t clientID, float& value)
    {
        _gStrength = value;
        _updateStrength = true;

        RecalculateResultantMatrix();
    };
    auto BChangeCB = [this] (uint32_t clientID, float& value)
    {
        _bStrength = value;
        _updateStrength = true;

        RecalculateResultantMatrix();
    };
    _spRStrength = spContainer->AddSliderControl("Red Channel Strength", _rStrength, 0.0, 2.0, RChangeCB, 2);
    _spGStrength = spContainer->AddSliderControl("Green Channel Strength", _gStrength, 0.0, 2.0, GChangeCB, 2);
    _spBStrength = spContainer->AddSliderControl("Blue Channel Strength", _bStrength, 0.0, 2.0, BChangeCB, 2);

    auto ContrastChangeCB = [this] (uint32_t clientID, float& value)
    {
        _contrast = value;
        _updateContrastAndFloor = true;

        RecalculateResultantMatrix();
    };
    auto FloorRaiseCB = [this] (uint32_t clientID, float& value)
    {
        _floor = value;
        _updateContrastAndFloor = true;

        RecalculateResultantMatrix();
    };
    _spContrastSlider = spContainer->AddSliderControl("Contrast Adjustment", _contrast, 0, 2.0, ContrastChangeCB, 2);
    _spImageFloorSlider = spContainer->AddSliderControl("Raise image floor", _floor, 0, 1.0, FloorRaiseCB, 2);


    if (_enableDebugUi)
    {
        auto summScaleCB = [this] (uint32_t clientID, int32_t& value)
        {
            _summand_scale = value;
            _updateContrastAndFloor = true;

            RecalculateResultantMatrix();
        };
        _spSummScaleSlider = spContainer->AddSliderControl("Summand scale", 0, 31, summScaleCB, "summandScaleSlider", _summand_scale);
    }

    // TODO - make this a proper reset function instead of having two sets of initialisations
    auto resetButtonCB = [this](uint32_t clientID)
    {
        ResetParamsAndRecalculate();
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetButtonCB, "Reset", "OJL/Images/Reset.png" }
    });
    
    return {std::move(spContainer)};
}
