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
#include "IWebSocketHandler.h"

class AIResultsControlsDisplayValue
{
public:
    AIResultsControlsDisplayValue(bool display);
    AIResultsControlsDisplayValue();
    bool _display;
    std::string ToString(bool csvFormat, int32_t firstIndex = -1);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    static AIResultsControlsDisplayValue FromString(bool csvFormat, std::string stringValue);
};

class AIRuntimeControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    AIRuntimeControls(const std::shared_ptr<SwApi::ICoreDlaRuntime>& spCoreDlaRuntime, const std::shared_ptr<SwApi::IAiResultsRenderer>& spIAiResultsRenderer, IWebSocketHandler* webSocketHandler, bool powerUser = false);
    virtual ~AIRuntimeControls();

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "AIRuntimeControls"; };

    AtUtils::JsonValueVariant UpdateValue(bool value,
                            bool callCallback = false,
                            uint32_t fromClientId = UiUpdate::invalidClient);
private:
    static const float _defaultDetectionThresholdValue;
    static const float _defaultIOUThresholdValue;
    static const float _defaultKeypointThresholdValue;

    std::shared_ptr<SwApi::ICoreDlaRuntime>     _spCoreDlaRuntime;
    std::shared_ptr<SwApi::IAiResultsRenderer>  _spIAiResultsRenderer;
    IWebSocketHandler*                          _webSocketHandler;
    bool _powerUser;

    std::shared_ptr<UiControlItemEnum>          _spNetworkEnum;
    std::shared_ptr<UiControlItemSlider>        _spDetectionThreshold;
    std::shared_ptr<UiControlItemSlider>        _spIOUThreshold;
    std::shared_ptr<UiControlItemSlider>        _spKeypointThreshold;
    std::shared_ptr<UiControlItemBoolean>       _spDisplayResultsOverlay;
    std::shared_ptr<UiControlItemBoolean>       _spDisplayResultsHere;
    std::shared_ptr<UiCustomControlWithValue<AIResultsControlsDisplayValue>> _spAIResultsControl;
    std::shared_ptr<UiControlItemBoolean>        _spDiagnosticsControl;

    NetworkType _networkType;
    float _detectionThreshold;
    float _iouThreshold;
    float _keypointThreshold;
    bool _displayResultsOverlay;
    bool _displayResultsHere;
    bool _diagnostics;
    size_t _websocket_handler_handle;
    bool _display;
};
