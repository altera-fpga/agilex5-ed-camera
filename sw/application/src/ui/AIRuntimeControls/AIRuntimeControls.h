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
#include "ICoreDlaRuntime.h"
#include "IAiResultsRenderer.h"

class AIRuntimeControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    AIRuntimeControls(const std::shared_ptr<SwApi::ICoreDlaRuntime>& spCoreDlaRuntime, const std::shared_ptr<SwApi::IAiResultsRenderer>& spIAiResultsRenderer, bool powerUser = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspCoredlaRuntime"; };

private:
    static const float _defaultDetectionThresholdValue;
    static const float _defaultIOUThresholdValue;
    static const float _defaultKeypointThresholdValue;

    std::shared_ptr<SwApi::ICoreDlaRuntime>     _spCoreDlaRuntime;
    std::shared_ptr<SwApi::IAiResultsRenderer>  _spIAiResultsRenderer;
    bool _powerUser;

    std::shared_ptr<UiControlItemEnum>          _spNetworkEnum;
    std::shared_ptr<UiControlItemSlider>        _spDetectionThreshold;
    std::shared_ptr<UiControlItemSlider>        _spIOUThreshold;
    std::shared_ptr<UiControlItemSlider>        _spKeypointThreshold;
    std::shared_ptr<UiControlItemBoolean>       _spDisplayResults;
    std::shared_ptr<UiControlItemButton>        _spDefaultsButton;
    std::shared_ptr<UiControlItemBoolean>        _spDiagnosticsControl;

    NetworkType _networkType;
    float _detectionThreshold;
    float _iouThreshold;
    float _keypointThreshold;
    bool _displayResults;
    bool _diagnostics;
};
