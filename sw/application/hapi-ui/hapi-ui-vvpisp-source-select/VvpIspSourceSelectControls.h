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

class VvpIspSourceSelectControls : public IpUiControls
{
public:

    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspSourceSelectControls(const std::string& name,
                                const std::vector<std::string>& inputOpts,
                                std::function<void(const uint32_t)> inputSelectCb);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspSourceSelect"; };

    void ApplySelectedInput();

private:
    std::string _name;
    std::vector<std::string> _inputOpts;
    std::function<void(const uint32_t)> _inputSelectCb;
    std::shared_ptr<UiControlItemEnum> _spInputSelectControl;
};
