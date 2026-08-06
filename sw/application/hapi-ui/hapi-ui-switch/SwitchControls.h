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
#include "ISwitch.h"

/// Encapsulates UI functionality for the Switch IP
class SwitchControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    SwitchControls(const std::shared_ptr<SwApi::ISwitch>& spSwitch,
                    const std::string& name = "Switch");
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "Switch"; };
private:
    std::shared_ptr<SwApi::ISwitch> _spSwitch;
    std::string _name;

    std::shared_ptr<UiControlItemEnum> _spInputSelectControl;
};
