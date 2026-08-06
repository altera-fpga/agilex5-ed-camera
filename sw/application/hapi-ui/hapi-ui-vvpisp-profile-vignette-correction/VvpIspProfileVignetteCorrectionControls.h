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
#include "Vc.h"
#include "HistogramStats.h"
#include "VcMeshUtils.h"

class VvpIspProfileVignetteCorrectionControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspProfileVignetteCorrectionControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                            const std::shared_ptr<WhiteBalanceController>& spWBController,
                                            const std::shared_ptr<SwApi::Vc>& spVc,
                                            const std::shared_ptr<SwApi::HistogramStats>& spHs);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspProfileVignetteCorrection"; };

    enum MeshGenerationMode
    {
        SampleCorrect = 0,
        DemoAmplifyVignette,
        DemoHorizontalGradient,
        DemoVerticalGradient,
    };

    void LoadFunctionPointerForUpdatingOverviewUI(std::function<void(void)> fpUpdateOverviewUiCB);

private:

    uint32_t _setXDim = 17;
    uint32_t _setYDim = 17;

    MeshGenerationMode _selectedMode = MeshGenerationMode::SampleCorrect;

    std::shared_ptr<SensorCalibrationProfile> _spProfile;
    std::shared_ptr<WhiteBalanceController> _spWBController;
    std::shared_ptr<SwApi::Vc> _spVc;
    std::shared_ptr<SwApi::HistogramStats> _spHs;

    void GenerateAndApplyVCMesh();
    SwApi::VcMeshUtils::VcCpFloatMesh SampleScreenWithHS(uint16_t width, uint16_t height, SwApi::VcMeshUtils::VcStepSampleCoordMesh sampleMesh);

    std::shared_ptr<UiControlItemButton> _spGenerateButton;

    std::function<void(void)> _fpUpdateOverviewUiCB;

};
