/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <filesystem>
#include "IpUiControls.h"
#include "ILut3d.h"

class Lut3dControls : public IpUiControls
{
public:
    Lut3dControls(const std::shared_ptr<SwApi::ILut3d>& spLut3d, const std::string& title = "3D LUT", const std::string& settingName = "ThreeDLUT");
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { return _settingName; }

private:
    std::filesystem::path GetCubeFilePath();
    void UpdateActiveLutButtonLabel();
    std::shared_ptr<SwApi::ILut3d> _spLut3d;
    std::shared_ptr<UiControlItemBoolean> _enableCheckbox;
    std::shared_ptr<UiControlItemButton> _spSelectLutButtons[2];
    std::shared_ptr<ISettingValue<std::string>> _spLutNameSetting;
    std::string _title;
    std::string _settingName;
};