/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <cstdint>
#include "IpUiControls.h"

class OutputControls : public IpUiControls
{
public:

    /// Creates an instance of the class that'll manage the given Switch instance.
    OutputControls(const std::string& name,
                                const std::vector<std::string>& sourceOpts,
                                std::function<void(const uint32_t)> sourceCb,
                                const std::vector<std::string>& overrideOpts,
                                std::function<void(const uint32_t, const bool)> overrideCb,
                                bool powerUser);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "OutputControls"; };

    void UpdateStatus(const uint32_t status, const std::string& format_str, const std::vector<uint32_t>& format_opts, const bool bpc10_support);

private:
    std::string _name;

    std::vector<std::string> _sourceOpts;    
    std::function<void(const uint32_t)> _sourceCb;
    std::shared_ptr<UiControlItemEnum> _spSourceSelectControl;

    std::shared_ptr<UiControlItemLabel> _spCurrentFormatLabel;
    std::shared_ptr<UiControlItemLabel> _spStatusLabel;
    std::vector<std::string> _overrideOpts;
    std::function<void(const uint32_t, const bool)> _overrideCb;
    std::shared_ptr<UiControlItemEnum> _spOverrideControl;

    bool _bpc10Support;
    bool _powerUser;
};