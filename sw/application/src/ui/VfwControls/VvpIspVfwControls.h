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
#include "VideoFrameWriter.h"
#include "IFrameCapture.h"


class VvpIspVfwControls : public IpUiControls
{
public:
    VvpIspVfwControls(std::shared_ptr<IFrameCapture> spIFrameCapture, const std::string& name = "Frame Writer");

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
    std::string GetSettingsSectionName() override { return "VvpIspVfwControls"; };

private:
    void GenerateImageFile(const SwApi::vfw_frame_t& frame);

    std::shared_ptr<IFrameCapture> _spIFrameCapture;
    std::string _name;
    std::shared_ptr<UiControlItemEnum> _spInputSelectControl;
};
