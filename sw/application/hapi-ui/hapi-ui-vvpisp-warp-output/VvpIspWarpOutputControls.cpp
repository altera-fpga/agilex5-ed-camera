/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspWarpOutputControls.h"
#include "UiElements.h"

#include <string>

VvpIspWarpOutputControls::VvpIspWarpOutputControls()
{
}

void VvpIspWarpOutputControls::LoadCallback(std::function<void(WarpOutputSetting)> fpPipelineWarpSetOutputCB)
{
    _fpPipelineWarpSetOutputCB = std::move(fpPipelineWarpSetOutputCB);
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspWarpOutputControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Warp Output", GetSettingsSectionName());
    std::vector<UiEnumOption> warpOutputResolutionOptions =
    {
        { "Follow Output", (uint32_t)WarpOutputSetting::FOLLOW_OUTPUT },
        { "1920x1080", (uint32_t)WarpOutputSetting::W1920x1080 },
        { "3840x2160", (uint32_t)WarpOutputSetting::W3840x2160 }
    };

    auto warpOutputResolutionDropDownCB = [this](uint32_t clientID, const UiEnumOption& value, uint32_t)
    {
        if (_fpPipelineWarpSetOutputCB)
        {
            _fpPipelineWarpSetOutputCB((WarpOutputSetting)value._userItemData);
        }
    
    };

    _warpOutputResolutionDropdown = spContainer->AddEnumControl("Set output",
                                                                    (uint32_t)WarpOutputSetting::FOLLOW_OUTPUT,
                                                                    warpOutputResolutionOptions,
                                                                    warpOutputResolutionDropDownCB);
    return {std::move(spContainer)};
}
