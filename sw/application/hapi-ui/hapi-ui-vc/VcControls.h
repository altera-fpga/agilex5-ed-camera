/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "Vc.h"
#include "IpUiControls.h"

/// Encapsulates UI functionality for the Vignette Correction IP
class VcControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given VC instance.
    VcControls(const std::string& name, const std::shared_ptr<SwApi::Vc>& spVc, bool enableDebugUi = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void GenerateAndApplyNewMesh();

private:
    const std::string _name;
    std::shared_ptr<SwApi::Vc> _spVc;

    std::shared_ptr<UiControlItemBoolean> _spBypassControl;
    std::shared_ptr<UiControlItemSlider> _spRadialSlider;
    std::shared_ptr<UiControlItemSlider> _spCoronaSlider;
    std::shared_ptr<UiControlItemBoolean> _spInvertBool;

    std::shared_ptr<UiControlItemLabel> _spFrameStats;

    float _radialDistance = 0.0f;
    float _coronaStrength = 0.0f;
    bool _invertMesh = false;

    bool _enableDebugUi = false;
};
