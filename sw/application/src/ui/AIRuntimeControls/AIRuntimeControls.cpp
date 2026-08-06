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

#include <string>

const float AIRuntimeControls::_defaultDetectionThresholdValue = 0.5f;
const float AIRuntimeControls::_defaultIOUThresholdValue = 0.85f;
const float AIRuntimeControls::_defaultKeypointThresholdValue = 0.5f;

AIRuntimeControls::AIRuntimeControls(const std::shared_ptr<SwApi::ICoreDlaRuntime>& spCoreDlaRuntime, const std::shared_ptr<SwApi::IAiResultsRenderer>& spIAiResultsRenderer, bool powerUser)
: _spCoreDlaRuntime(spCoreDlaRuntime)
, _spIAiResultsRenderer(spIAiResultsRenderer)
, _powerUser(powerUser)
, _networkType(NetworkType::YOLOV8N)
, _detectionThreshold(_defaultDetectionThresholdValue)
, _iouThreshold(_defaultIOUThresholdValue)
, _keypointThreshold(_defaultKeypointThresholdValue)
, _displayResults(true)
, _diagnostics(false)
{
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
    
    auto displayResultsCB = [this](uint32_t clientID, bool& val)
    {
        _displayResults = val;
        _spIAiResultsRenderer->RenderResults(_displayResults);
    };
    _spDisplayResults = spContainer->AddBoolControl("Display Results", true, displayResultsCB);

    if(_powerUser)
    {
        auto diagnosticsCB = [this](uint32_t clientID, bool &val)
        {
            if (_spCoreDlaRuntime)
            {
                _diagnostics = val;
                _spCoreDlaRuntime->SetDiagnostics(_diagnostics);
            }
        };

        _spDiagnosticsControl = spContainer->AddBoolControl("Diagnostics", false, diagnosticsCB);
    }
   
    auto defaultsButtonCB = [this](uint32_t clientID) 
    {
        _spDetectionThreshold->UpdateValue(_defaultDetectionThresholdValue, true);
        _spIOUThreshold->UpdateValue(_defaultIOUThresholdValue, true);
        _spKeypointThreshold->UpdateValue(_defaultKeypointThresholdValue, true);
        _spDisplayResults->UpdateValue(true, true);
        if(_powerUser)
        {
            _spDiagnosticsControl->UpdateValue(false, true);
        }
    };
    _spDefaultsButton = spContainer->AddButtonControl("Defaults", defaultsButtonCB);

    return {std::move(spContainer)};
}
