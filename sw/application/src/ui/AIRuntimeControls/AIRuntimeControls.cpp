/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "AIRuntimeControls.h"
#include "UiElements.h"
#include "IWebSocketHandler.h"

#include <string>

const float AIRuntimeControls::_defaultDetectionThresholdValue = 0.5f;
const float AIRuntimeControls::_defaultIOUThresholdValue = 0.85f;
const float AIRuntimeControls::_defaultKeypointThresholdValue = 0.5f;

AIRuntimeControls::AIRuntimeControls(const std::shared_ptr<SwApi::ICoreDlaRuntime>& spCoreDlaRuntime, const std::shared_ptr<SwApi::IAiResultsRenderer>& spIAiResultsRenderer, IWebSocketHandler* webSocketHandler, bool powerUser)
: _spCoreDlaRuntime(spCoreDlaRuntime)
, _spIAiResultsRenderer(spIAiResultsRenderer)
, _webSocketHandler(webSocketHandler)
, _powerUser(powerUser)
, _networkType(NetworkType::YOLOV8N)
, _detectionThreshold(_defaultDetectionThresholdValue)
, _iouThreshold(_defaultIOUThresholdValue)
, _keypointThreshold(_defaultKeypointThresholdValue)
, _displayResultsOverlay(true)
, _displayResultsHere(true)
, _diagnostics(false)
, _websocket_handler_handle(-1)
{
    if(_webSocketHandler)
    {
        _websocket_handler_handle = _webSocketHandler->RegisterWebSocketHandler([this](IWebSocketService* web_socket) -> bool
        {
            bool handled = false;
            std::string subProtocolString = web_socket->GetSubProtocolString();
            if(subProtocolString == "ai-results")
            {
                if(_spIAiResultsRenderer)
                {
                    handled = _spIAiResultsRenderer->ConnectWebSocketService(web_socket);
                }
            }

            return handled;
        });
    }
}

AIRuntimeControls::~AIRuntimeControls()
{
    if(_webSocketHandler)
    {
        _webSocketHandler->UnRegisterWebSocketHandler(_websocket_handler_handle);
    }
}

std::vector<std::shared_ptr<UiControlContainer>> AIRuntimeControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("AI Runtime", GetSettingsSectionName());
    auto network_list = _spCoreDlaRuntime->GetNetworkList();

    auto networkCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
    {
        uint32_t selected_network_handle = selected._userItemData;
        auto network_list = _spCoreDlaRuntime->GetNetworkList();
        ModelPtr network_model;
        for (const auto& it_network_model: network_list)
        {
            if(it_network_model)
            {
                if(it_network_model->_network_handle == selected_network_handle)
                {
                    network_model = it_network_model;
                }
            }
        }
        if(network_model)
        {
            _networkType = network_model->_network_type;
            _spCoreDlaRuntime->SetNetworkHandle(network_model->_network_handle);
        }
        else
        {
            _networkType = NetworkType::UNKNOWN;
            _spCoreDlaRuntime->SetNetworkHandle(0);
        }
        if(_spKeypointThreshold)
        {
            if(_networkType == NetworkType::YOLOV8N_POSE)
            {
                _spKeypointThreshold->Enable(true);
            }
            else
            {
                _spKeypointThreshold->Enable(false);
            }
        }
    };

    std::vector<UiEnumOption> networks;
    for(auto network_model : network_list)
    {
        networks.emplace_back(network_model->_network_name, network_model->_network_handle);
    }
    _spNetworkEnum = spContainer->AddEnumControl("Network",
                                                networks,
                                                networkCB,
                                                "CoreDLA Network",
                                                0);

    auto detectionThresholdCB = [this](uint32_t clientID, float& val)
    {
        _detectionThreshold = val;
        _spCoreDlaRuntime->SetDetectionThreshold(_detectionThreshold);
    };
    _spDetectionThreshold = spContainer->AddSliderControl("Detection Threshold", 0.05f, 1.0f, detectionThresholdCB, "Detection Threshold", _defaultDetectionThresholdValue);

    auto iouThresholdCB = [this](uint32_t clientID, float& val)
    {
        _iouThreshold = val;
        _spCoreDlaRuntime->SetIOUThreshold(_iouThreshold);
    };
    _spIOUThreshold = spContainer->AddSliderControl("IOU Threshold", 0.0f, 1.0f, iouThresholdCB, "IOU Threshold", _defaultIOUThresholdValue);

    auto keypointThresholdCB = [this](uint32_t clientID, float& val)
    {
        _keypointThreshold = val;
        _spIAiResultsRenderer->SetKeypointThreshold(_keypointThreshold);
    };
    _spKeypointThreshold = spContainer->AddSliderControl("Keypoint Threshold", 0.0f, 1.0f, keypointThresholdCB, "Keypoint Threshold", _defaultKeypointThresholdValue);
    if(_networkType == NetworkType::YOLOV8N_POSE)
    {
        _spKeypointThreshold->Enable(true);
    }
    else
    {
        _spKeypointThreshold->Enable(false);
    }
    
    auto displayResultsOverlayCB = [this](uint32_t clientID, bool& val)
    {
        _displayResultsOverlay = val;
        _spIAiResultsRenderer->RenderResults(_displayResultsOverlay);
    };
    _spDisplayResultsOverlay = spContainer->AddBoolControl("Display Results Overlay", true, displayResultsOverlayCB);

    auto displayResultsHereCB = [this](uint32_t clientID, bool& val)
    {
        _displayResultsHere = val;
        if(_spAIResultsControl)
        {
            _spAIResultsControl->UpdateValue(_displayResultsHere, true);
        }
    };
    _spDisplayResultsHere = spContainer->AddBoolControl("Web-UI Display Results", true, displayResultsHereCB);

    auto aiResultsControlCB = [this](uint32_t clientId, AIResultsControlsDisplayValue& value)
    {
    };
    _spAIResultsControl = std::make_shared<UiCustomControlWithValue<AIResultsControlsDisplayValue>>("OJAIResultsControl", AIResultsControlsDisplayValue(true), aiResultsControlCB);
    spContainer->Add(_spAIResultsControl);

    auto diagnosticsCB = [this](uint32_t clientID, bool &val)
    {
        if (_spCoreDlaRuntime)
        {
            _diagnostics = val;
            _spCoreDlaRuntime->SetDiagnostics(_diagnostics);
        }
    };

    _spDiagnosticsControl = spContainer->AddBoolControl("Display Statistics Terminal", false, diagnosticsCB);
   
    auto defaultsButtonCB = [this](uint32_t clientID) 
    {
        _spDetectionThreshold->UpdateValue(_defaultDetectionThresholdValue, true);
        _spIOUThreshold->UpdateValue(_defaultIOUThresholdValue, true);
        _spKeypointThreshold->UpdateValue(_defaultKeypointThresholdValue, true);
        _spDisplayResultsOverlay->UpdateValue(true, true);
        _spDisplayResultsHere->UpdateValue(true, true);
        if(_powerUser)
        {
            _spDiagnosticsControl->UpdateValue(false, true);
        }
    };

    auto resetCB = [this](uint32_t clientID) 
    {
        _spDetectionThreshold->UpdateValue(_defaultDetectionThresholdValue, true);
        _spIOUThreshold->UpdateValue(_defaultIOUThresholdValue, true);
        _spKeypointThreshold->UpdateValue(_defaultKeypointThresholdValue, true);
        _spDisplayResultsOverlay->UpdateValue(true, true);
        _spDisplayResultsHere->UpdateValue(true, true);
        if(_powerUser)
        {
            _spDiagnosticsControl->UpdateValue(false, true);
        }
    };
    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetCB, "Reset", "OJL/Images/Reset.png" }
    });


    return {std::move(spContainer)};
}

AIResultsControlsDisplayValue::AIResultsControlsDisplayValue(bool display)
:    _display(display)
{
}

AIResultsControlsDisplayValue::AIResultsControlsDisplayValue()
:    _display(false)
{
}

std::string AIResultsControlsDisplayValue::ToString(bool csvFormat, int32_t firstIndex)
{
    if (csvFormat)
    {
        std::string stringValue = AtUtils::FormatString("%d", _display);
        return stringValue;
    }
    else
    {
        auto spJson = AtUtils::IJson::Create();
        if (!spJson)
            return {};

        auto spObject = spJson->RootObject();
        if (!spObject)
            return {};

        spObject->AddValue("display", (int32_t)_display);

        return spJson->ToString();
    }
}

void AIResultsControlsDisplayValue::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("display", (int32_t)_display);
}

AIResultsControlsDisplayValue AIResultsControlsDisplayValue::FromString(bool csvFormat, std::string stringValue)
{
    AIResultsControlsDisplayValue value;
    if (csvFormat)
    {
        std::string displayStr = UiElement::CsvExtract(stringValue);

        bool display = AtUtils::FromString<bool>(displayStr);
        value._display = display;

        return value;
    }
    else
    {
        auto spJson = AtUtils::IJson::Create(stringValue);
        if (!spJson)
            return {};

        auto spObject = spJson->Parse();
        if (!spObject)
            return {};

        bool display = (bool)spObject->GetValue<bool>("display");

        value._display = display;
    }

    return value;
}

