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

#include "CalibrationProfile.h"
#include "WhiteBalance.h"

#include "IspCommon.h"

class VvpIspProfileLoaderControls : public IpUiControls
{

public:

    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspProfileLoaderControls(const std::string& name,
                                const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                const std::shared_ptr<WhiteBalanceController>& spWBController);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name + "Preview"; }

    void LoadProfile(const std::string& profileTitle, const std::filesystem::path& importDestinationPath);
    void ProfileLoadedExternally(const std::string& title);

private:

    std::shared_ptr<SensorCalibrationProfile> _spProfile;
    std::shared_ptr<WhiteBalanceController> _spWBController;

    std::string _name;

    std::shared_ptr<UiControlItemLabel> _spCurrentProfileFeedbackLabel;
    std::shared_ptr<UiControlItemLabel> _spCurrentProfileVersionLabel;
    std::shared_ptr<UiControlItemLabel> _spCurrentProfileOptionStatusLabel;

};
