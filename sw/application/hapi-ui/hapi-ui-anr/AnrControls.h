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
#include "Anr.h"
#include "CalibrationProfile.h"

/// Encapsulates UI functionality for the Adaptive Noise Reduction IP
class AnrControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given ANR instance.
    AnrControls(const std::string& name, const std::shared_ptr<SwApi::Anr>& spAnr);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

	std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

private:
    const std::string _name;
    std::shared_ptr<SwApi::Anr> _spAnr;
    
    bool _ready = false;

    std::shared_ptr<UiControlItemBoolean>   _spBypass;
    std::shared_ptr<UiControlItemSlider>    _spStrength;

    float _combinedGain = 1.0f;
    float _darkNoise = 1.0f;
    float _strength = 1.0f;
};
