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
#include <memory>
#include <filesystem>
#include <functional>
#include "IpUiControls.h"
#include "IspVfrInput.h"


class UiControlItemLabel;


class VvpIspFrameReaderControls : public IpUiControls
{
public:
    explicit VvpIspFrameReaderControls(std::shared_ptr<SwApi::IspVfrInput> spVfrInput);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
    void UpdateSourceMetadata(const SwApi::VfrSourceMetadata& metadata);
    std::string GetSettingsSectionName() override { return "VvpIspFrameReader"; }

private:
    std::shared_ptr<UiControlItemLabel> _spCurrentFileLabel;
    std::shared_ptr<UiControlItemLabel> _spResolutionLabel;
    std::shared_ptr<UiControlItemLabel> _spBitsPerPixelLabel;
    std::shared_ptr<UiControlItemLabel> _spSourceTypeLabel;

    std::shared_ptr<SwApi::IspVfrInput> _spVfrInput;
};
