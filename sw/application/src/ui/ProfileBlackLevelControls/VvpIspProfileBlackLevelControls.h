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
#include "Bls.h"
#include "Anr.h"
#include "ICamera.h"
#include "IFrameCapture.h"


class VvpIspProfileBlackLevelControls : public IpUiControls
{
public:
    VvpIspProfileBlackLevelControls(const std::weak_ptr<IFrameCapture>& spIFrameCapture,
                                    const std::shared_ptr<SensorCalibrationProfile>& spProfile, 
                                    const std::shared_ptr<WhiteBalanceController>& spWBController,
                                    const ICameraPtr& spCamera,
                                    const std::shared_ptr<SwApi::Bls>& spBlackLevelStats,
                                    const std::shared_ptr<SwApi::Anr>& spAnr);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspProfileBlackLevel"; };

    void UpdateUiWithCurrentProfile();

    void LoadFunctionPointerForUpdatingOverviewUI(std::function<void(void)> fpUpdateOverviewUiCB);

private:

    void ReadBLSAndUpdateUI();
    void MacroControlReadBlackLevels();
    void AddNewGain(float gain);
    void DeleteGain(float gain);
    void SelectGain(float gain);
    void ResetGainDropdownAndSelectGain(float gain);

    std::weak_ptr<IFrameCapture> _wspIFrameCapture;
    std::shared_ptr<SensorCalibrationProfile> _spProfile;
    std::shared_ptr<WhiteBalanceController> _spWBController;
    ICameraPtr _spCamera;
    std::shared_ptr<SwApi::Bls> _spBlackLevelStats;
    std::shared_ptr<SwApi::Anr> _spAnr;

    float _selectedGain = 1.0;
    float _enteredGain = 1.0;

    std::shared_ptr<UiControlItemEnum> _spGainSelectEnum;

    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Ped00Int;
    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Ped01Int;
    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Ped10Int;
    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Ped11Int;

    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Scl00Int;
    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Scl01Int;
    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Scl10Int;
    std::shared_ptr<UiControlItemUInteger> _spBLCtrl_Scl11Int;

    std::shared_ptr<UiControlItemButton> _spReadBLSButton;
    std::shared_ptr<UiControlItemButton> _spMacroReadBLSButton;

    std::shared_ptr<UiControlItemFloat> _spAnrCombinedGain;
    std::shared_ptr<UiControlItemFloat> _spAnrDarkNoise;
    std::shared_ptr<UiControlItemButton> _spReadDarkFrameButton;
    std::shared_ptr<UiControlItemButton> _spReadSceneFramesButton;
    std::shared_ptr<UiControlItemButton> _spCalculateParamsButton;

    float _combinedGain = 0.0f;
    float _darkNoise = 0.0f;

    using frame_data_t = SwUtils::mem_buffer_t;

    frame_data_t _darkFrame;
    std::vector<frame_data_t> _sceneFrames;
 
    std::function<void(void)> _fpUpdateOverviewUiCB;

};
