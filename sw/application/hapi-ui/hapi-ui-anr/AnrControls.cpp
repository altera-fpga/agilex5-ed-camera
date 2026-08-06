/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "AnrControls.h"
#include "UiElements.h"
#include "intel_vvp_anr.h"

using namespace SwApi;

static constexpr float DEFAULT_STRENGTH = 0.5f;

AnrControls::AnrControls(const std::string& name, const std::shared_ptr<SwApi::Anr>& spAnr)
    : _name(name)
    , _spAnr(spAnr)
{
}

std::vector<std::shared_ptr<UiControlContainer>> AnrControls::AddUiElements()
{
    if (not _spAnr) {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    auto bypassCB = [this](uint32_t clientID, bool& val)
    {
        // When starting up push any persisted settings to the hardware
        // regardless of the pending commit status
        auto updatePolicy = _ready ? UpdatePolicy::Sync() : UpdatePolicy::Async();
        _spAnr->SetBypass(val, updatePolicy);
    };

    _spBypass = spContainer->AddBoolControl("Bypass", bypassCB, "BypassANR", _spAnr->GetBypass());

    auto strengthCB = [this](uint32_t clientID, float& val)
    {
        _strength = val;
        auto updatePolicy = _ready ? UpdatePolicy::Sync() : UpdatePolicy::Async();
        _spAnr->ApplyLuts(_strength, updatePolicy);
    };
    _spStrength = spContainer->AddSliderControl("Strength", 0.0f, 2.0f, strengthCB, "StrengthANR", DEFAULT_STRENGTH);

    auto resetButtonCB = [this](uint32_t clientID)
    {
        _spStrength->UpdateValue(DEFAULT_STRENGTH, true);
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetButtonCB, "Reset", "OJL/Images/Reset.png" }
    });

    _ready = true;

    return {std::move(spContainer)};
}
