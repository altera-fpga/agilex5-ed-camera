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
#include "IDemosaic.h"

/// Encapsulates UI functionality for the Demosaic IP
class DemosaicControls : public IpUiControls
{
public:

    /// Creates an instance of the class that'll manage the given Demosaic instance.
    DemosaicControls(const std::string& name, 
                     const std::shared_ptr<SwApi::IDemosaic>& spDemosaic,
                     bool enableDebugUi);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void StatsUpdateLoop();
private:
    const std::string _name; 
    std::shared_ptr<SwApi::IDemosaic> _spDemosaic;
    bool _enableDebugUi;

    std::shared_ptr<UiControlItemBoolean> _spBypassControl;
    std::shared_ptr<UiControlItemEnum> _spColorFilterArraySelectControl;

    std::shared_ptr<UiControlItemLabel> _spFrameStats;
};
