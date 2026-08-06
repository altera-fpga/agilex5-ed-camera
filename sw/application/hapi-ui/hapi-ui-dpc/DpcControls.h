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
#include "IDpc.h"

/// Encapsulates UI functionality for the Defective Pixel Correction IP
class DpcControls : public IpUiControls
{
public:

    /// Creates an instance of the class that'll manage the given DPC instance.
    DpcControls(const std::string& name, 
                const std::shared_ptr<SwApi::IDpc>& spDpc,
                bool enableDebugUi = false);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void StatsUpdateLoop();

private:
    const std::string _name;
    std::shared_ptr<SwApi::IDpc> _spDpc;

    std::shared_ptr<UiControlItemBoolean> _spBypassControl;
    std::shared_ptr<UiControlItemEnum>    _spSensitivityControl;
    std::shared_ptr<UiControlItemEnum>    _spCfaPhaseControl;

    std::shared_ptr<UiControlItemLabel> _spFrameStats;
    std::shared_ptr<UiControlItemLabel> _spPixLoCounter;
    std::shared_ptr<UiControlItemLabel> _spPixHiCounter;

    bool _enableDebugUi = false;
};
