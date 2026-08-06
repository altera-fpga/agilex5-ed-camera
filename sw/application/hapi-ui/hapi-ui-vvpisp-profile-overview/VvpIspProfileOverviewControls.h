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

class VvpIspProfileOverviewControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspProfileOverviewControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspProfileOverview"; };

    void UpdateUiWithCurrentProfile();
    void AddProfileLoadUiUpdateCallback(std::function<void(std::string)> callback);


private:

    void ExportProfile(uint32_t clientId);

    std::shared_ptr<SensorCalibrationProfile> _spProfile;

    std::shared_ptr<UiControlItemText> _spSensorTextbox;
    std::shared_ptr<UiControlItemLabel> _spProfileVersionLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileValidLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileNumAnalogueGainsLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileNumLumSamplesLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileBaselineTempLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileNumTempSamplesLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileANRCoeffsPresentLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileVCMeshPresentLabel;
    std::shared_ptr<UiControlItemLabel> _spProfileVCMeshDimensionLabel;

    std::function<void(std::string)> _updateAllCalibUiCB;
};
