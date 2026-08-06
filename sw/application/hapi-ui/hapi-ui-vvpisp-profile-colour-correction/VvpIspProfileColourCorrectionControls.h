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

#include "WhiteBalance.h"
#include "CalibrationProfile.h"
#include "IspCommon.h"
#include "ICsc.h"

class VvpIspProfileColourCorrectionControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspProfileColourCorrectionControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                      const std::shared_ptr<WhiteBalanceController>& spWBController,
                                      const std::shared_ptr<SwApi::ICsc>& spCcm);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspProfileColourCorrection"; };

    void UpdateUiWithCurrentProfile();

private:

    std::shared_ptr<SensorCalibrationProfile> _spProfile;
    std::shared_ptr<WhiteBalanceController> _spWBController;
    std::shared_ptr<SwApi::ICsc> _spCcm;

    std::shared_ptr<UiControlItemSlider> _spR0Slider;
    std::shared_ptr<UiControlItemSlider> _spR1Slider;
    std::shared_ptr<UiControlItemSlider> _spR2Slider;

    std::shared_ptr<UiControlItemSlider> _spG0Slider;
    std::shared_ptr<UiControlItemSlider> _spG1Slider;
    std::shared_ptr<UiControlItemSlider> _spG2Slider;

    std::shared_ptr<UiControlItemSlider> _spB0Slider;
    std::shared_ptr<UiControlItemSlider> _spB1Slider;
    std::shared_ptr<UiControlItemSlider> _spB2Slider;

    std::function<void(void)> _fpUpdateOverviewUiCB;
};
