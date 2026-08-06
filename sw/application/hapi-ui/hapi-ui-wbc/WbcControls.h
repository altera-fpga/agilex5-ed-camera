/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "Wbc.h"
#include "IpUiControls.h"

/// Encapsulates UI functionality for the White Balance Correction IP
class WbcControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given WBC instance.
    WbcControls(const std::string& name, const std::shared_ptr<SwApi::Wbc>& spWbc, bool enableDebugUi = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void BlockUIBasedOnAWBCallback(bool isAWBDrivingWbc);
    void BypassBasedOnAWBCallback(bool isBypass);
    void UpdateControlsFromHardware();

private:
    const std::string _name;
    std::shared_ptr<SwApi::Wbc> _spWbc;

    std::shared_ptr<UiControlItemBoolean> _spBypassControl;
    std::shared_ptr<UiControlItemEnum> _spCfaPhaseControl;

    std::shared_ptr<UiControlItemUInteger> _spScalar00;
    std::shared_ptr<UiControlItemUInteger> _spScalar01;
    std::shared_ptr<UiControlItemUInteger> _spScalar10;
    std::shared_ptr<UiControlItemUInteger> _spScalar11;

    bool _enableDebugUi = false;
};
