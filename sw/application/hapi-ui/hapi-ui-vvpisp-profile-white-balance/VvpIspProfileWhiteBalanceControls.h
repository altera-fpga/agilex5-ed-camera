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
#include "Wbc.h"

class VvpIspProfileWhiteBalanceControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspProfileWhiteBalanceControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                      const std::shared_ptr<WhiteBalanceController>& spWBController,
                                      const std::shared_ptr<SwApi::Wbc>& spWhiteBalanceCorrection,
                                      const std::shared_ptr<SwApi::Ccm>& spCcm);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspProfileWhiteBalance"; };

    void UpdateUiWithCurrentProfile();
    void LoadFunctionPointerForUpdatingOverviewUI(std::function<void(void)> fpUpdateOverviewUiCB);

private:

    uint16_t _enteredTemp = 6500;
    TCfaPhase _cfaPhase = TCfaPhase::RGGB;

    uint16_t _selectedTemp = 0;

    bool _tempBeenSelected = false;

    std::shared_ptr<SensorCalibrationProfile> _spProfile;
    std::shared_ptr<WhiteBalanceController> _spWBController;
    std::shared_ptr<SwApi::Wbc> _spWhiteBalanceCorrection;
    std::shared_ptr<SwApi::Ccm> _spColourCorrectionMatrix;

    void MacroReadBaselineWhiteBalance();
    void SelectTemp(uint16_t temp, TCfaPhase cfaPhase);
    void ResetTempDropdownAndSelectTemp(uint16_t temp);


    // Luminance Baseline
    std::shared_ptr<UiControlItemEnum> _spLumSelectEnum;
    std::shared_ptr<UiControlItemButton> _spLumAddButton;
    std::shared_ptr<UiControlItemButton> _spLumDelButton;
    std::shared_ptr<UiControlItemUInteger> _spLumBaselineInt;
    std::shared_ptr<UiControlItemUInteger> _spLumBaseCfa00Int;
    std::shared_ptr<UiControlItemUInteger> _spLumBaseCfa01Int;
    std::shared_ptr<UiControlItemUInteger> _spLumBaseCfa10Int;
    std::shared_ptr<UiControlItemUInteger> _spLumBaseCfa11Int;
    std::shared_ptr<UiControlItemButton> _spLumRecordButton;

    // Colour Temp Recording
    std::shared_ptr<UiControlItemUInteger> _spColTempInt;
    std::shared_ptr<UiControlItemEnum> _spColTempSelectEnum;
    std::shared_ptr<UiControlItemButton> _spAddTempButton;
    std::shared_ptr<UiControlItemButton> _spDelTempButton;
    std::shared_ptr<UiControlItemUInteger> _spColTempCfa00Int;
    std::shared_ptr<UiControlItemUInteger> _spColTempCfa01Int;
    std::shared_ptr<UiControlItemUInteger> _spColTempCfa10Int;
    std::shared_ptr<UiControlItemUInteger> _spColTempCfa11Int;
    std::shared_ptr<UiControlItemButton> _spReadColTempDataButton;


    // Colour Correction
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
