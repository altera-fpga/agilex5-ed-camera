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
#include "ICrs.h"

class CrsControls : public IpUiControls
{
public:
    CrsControls(const std::shared_ptr<SwApi::ICrs>& spCrs, bool enableDebugUi);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { return "ChromaResampling"; }

private:
    std::shared_ptr<SwApi::ICrs> _spCrs;
    bool _enableDebugUi = false;
    std::shared_ptr<UiControlItemEnum> _spInputSubsampling;
    std::shared_ptr<UiControlItemEnum> _spOutputSubsampling;
    std::vector<UiEnumOption> _subsamplingOptions = {
        {"4:2:0", (uint32_t)kIntelVvpCrsSubsampling420},
        {"4:2:2", (uint32_t)kIntelVvpCrsSubsampling422},
        {"4:4:4", (uint32_t)kIntelVvpCrsSubsampling444}
    };
};