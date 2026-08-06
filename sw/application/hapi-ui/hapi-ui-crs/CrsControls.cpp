/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CrsControls.h"

#ifdef __linux__
#include <unistd.h>
#endif


CrsControls::CrsControls(const std::shared_ptr<SwApi::ICrs>& spCrs, bool enableDebugUi)
: _spCrs(spCrs), _enableDebugUi(enableDebugUi)
{ 
    
}

std::vector<std::shared_ptr<UiControlContainer>> CrsControls::AddUiElements()
{
    if (!_spCrs)
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>("Chroma Resampler", GetSettingsSectionName());

    // Input subsampling selection drop down (DEBUG UI)
    if (_enableDebugUi) 
    {        
        auto refreshSubsamplingCB = [this](uint32_t clientID) -> void 
        {
            auto inSubsampling = _spCrs->GetInputSubsampling();
            switch (inSubsampling) 
            {
                case kIntelVvpCrsSubsampling420:
                {    
                    _spInputSubsampling->UpdateValue((uint32_t)kIntelVvpCrsSubsampling420, false);
                    break;
                }
                case kIntelVvpCrsSubsampling422:
                {                    
                    _spInputSubsampling->UpdateValue((uint32_t)kIntelVvpCrsSubsampling422, false);
                    break;
                }
                case kIntelVvpCrsSubsampling444:
                default:
                {
                    _spInputSubsampling->UpdateValue((uint32_t)kIntelVvpCrsSubsampling444, false);
                    break;
                }
            }

            auto outSubsampling = _spCrs->GetOutputSubsampling();
            switch (outSubsampling) 
            {
                case kIntelVvpCrsSubsampling420:
                {
                    _spOutputSubsampling->UpdateValue((uint32_t)kIntelVvpCrsSubsampling420, false);
                    break;
                }
                case kIntelVvpCrsSubsampling422:
                {
                    _spOutputSubsampling->UpdateValue((uint32_t)kIntelVvpCrsSubsampling422, false);
                    break;
                }
                case kIntelVvpCrsSubsampling444:
                default:
                {
                    _spOutputSubsampling->UpdateValue((uint32_t)kIntelVvpCrsSubsampling444, false);
                    break;
                }
            }
        };
        auto _spRefreshSubsampling = spContainer->AddButtonControl("Refresh Subsamping",
                                                                    refreshSubsamplingCB);
    }

    auto inputSubsamplingOptionsCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
    {
        auto chosenSampling = (eIntelCrsSubsampling)selected._userItemData;
        _spCrs->SetInputSubsampling(chosenSampling);
    };
    _spInputSubsampling = spContainer->AddEnumControl("Input Subsampling",
                                _subsamplingOptions,
                                inputSubsamplingOptionsCB,
                                "InputSampling",
                                (uint32_t)kIntelVvpCrsSubsampling444);

    // Output subsampling selection drop down
    auto outputSubsamplingOptionsCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
    {
        auto chosenSampling = (eIntelCrsSubsampling)selected._userItemData;
        _spCrs->SetOutputSubsampling(chosenSampling);
    };
    _spOutputSubsampling = spContainer->AddEnumControl("Output Subsampling",
                                                        _subsamplingOptions,
                                                        outputSubsamplingOptionsCB,
                                                        "OutputSampling",
                                                        (uint32_t)kIntelVvpCrsSubsampling444);

    return {std::move(spContainer)};
}
