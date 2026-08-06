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
#include "HapiVvpUsm.h"

/// Encapsulates UI functionality for the Black Level Statistics IP
class UsmControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given BLC instance.
    UsmControls(Hapi::VvpUsmPtr spUsm,
                bool enableDebugUi = true);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "UnSharpMaskingFilter"; };

private:

    Hapi::VvpUsmPtr _spUsm;

    std::shared_ptr<UiControlItemSlider> _spStrength;

    bool _enableDebugUi = false;
};
