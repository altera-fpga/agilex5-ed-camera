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
#include "VideoStandard.h"

enum class WarpOutputSetting {
  W1024x576,  // PAL
  W1280x720,  // HD
  W1440x900,  // WSXGA
  W1920x1080, // FullHD
  W2048x1080, // 2K
  W1600x1200, // UXGA
  W1920x1200, // WUXGA
  W2560x1600, // WQXGA
  W3264x2160, // Terasic max output
  W3440x1440, // UWQHD
  W3836x2156, // Test resolution
  W3840x2160, // UHD
  FOLLOW_OUTPUT,
};

class VvpIspWarpOutputControls: public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspWarpOutputControls();

    void LoadCallback(std::function<void(WarpOutputSetting)> fpPipelineWarpSetOutputCB);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspWarpOutputControl"; };


private:
    std::string _name;

    std::function<void(WarpOutputSetting)> _fpPipelineWarpSetOutputCB;

    std::shared_ptr<UiControlItemEnum>  _warpOutputResolutionDropdown;
};
