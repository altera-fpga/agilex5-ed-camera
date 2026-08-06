/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "UsmControls.h"
#include "UiElements.h"

UsmControls::UsmControls(Hapi::VvpUsmPtr spUsm,
                         bool enableDebugUi)
    : _spUsm(spUsm)
    , _enableDebugUi(enableDebugUi)
{
    if (_spUsm) {

    }
    else
    {

    }
}

std::vector<std::shared_ptr<UiControlContainer>> UsmControls::AddUiElements()
{

    auto spContainer = std::make_shared<UiControlContainer>("Unsharp Mask Filter",
                                                            GetSettingsSectionName());
    if (not _spUsm)
    {
        spContainer->AddLabelControl("Unsharp Mask Filter is not present");
        return {std::move(spContainer)};
    }

    auto usmUpdateUIStrength = [this](uint32_t clientID, float strength) -> void
    {
        int fp_strength = strength * (1 << 12);
        const unsigned int STRENGTH_MASK = 0x3FFF;

        intel_vvp_usm_set_sharpening_strength(_spUsm->GetInstance(), fp_strength & STRENGTH_MASK);
    };

    _spStrength = spContainer->AddSliderControl("Sharpening Strength", -1.99f, 1.99f,
        usmUpdateUIStrength, "strengthSlider", 1.0, 2u);

    auto resetCB = [this] (uint32_t clientID)
    {
        _spStrength->UpdateValue(1.0f, true);
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetCB, "Reset", "OJL/Images/Reset.png" }
    });

    return {std::move(spContainer)};
}