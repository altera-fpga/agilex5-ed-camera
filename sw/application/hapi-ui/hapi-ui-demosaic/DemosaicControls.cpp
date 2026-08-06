/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "DemosaicControls.h"
#include "IspCommon.h"
#include "UiElements.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

DemosaicControls::DemosaicControls(const std::string& name, 
                                   const std::shared_ptr<SwApi::IDemosaic>& spDemosaic,
                                   bool enableDebugUi)
    : _name(name),
      _spDemosaic(spDemosaic),
      _enableDebugUi(enableDebugUi)
{
    if (_spDemosaic) {
        // Initial setup here?
    } else {

    }
}

std::vector<std::shared_ptr<UiControlContainer>> DemosaicControls::AddUiElements() 
{
    if (not _spDemosaic) 
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());

    //region Enable/disable toggle
    auto enableCB = [this](uint32_t clientID, bool& val) -> void 
    {
        _spDemosaic->SetBypass(val);
    };
    _spBypassControl = spContainer->AddBoolControl("Bypass", enableCB, "BypassDemosaic", false);
    //endregion

    if (_enableDebugUi)
    {
        spContainer->AddLabelControl("");

        //region ColorFilterArray selection drop-down
        auto cfaCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
        {
            auto chosenCfaType = static_cast<TCfaPhase>(selected._userItemData);
            _spDemosaic->SetCfaPhase(chosenCfaType);
        };

        std::vector<UiEnumOption> cfaTypeOptions = 
        {
            { "RGGB", static_cast<uint8_t>(TCfaPhase::RGGB) },
            { "GRBG", static_cast<uint8_t>(TCfaPhase::GRBG) },
            { "GBRG", static_cast<uint8_t>(TCfaPhase::GBRG) },
            { "BGGR", static_cast<uint8_t>(TCfaPhase::BGGR) }
        };
        _spColorFilterArraySelectControl = spContainer->AddEnumControl("Bayer pattern type",
                                                                    cfaTypeOptions,
                                                                    cfaCB,
                                                                    "ColorFilterArray",
                                                                    0);
        //endregion
    
        _spFrameStats = spContainer->AddLabelControl("Checksum:", "");
    }

    return {std::move(spContainer)};
}

void DemosaicControls::StatsUpdateLoop()
{
    if (_enableDebugUi)
    {
        uint32_t stats;
        _spDemosaic->GetFrameStats(&stats);
        _spFrameStats->UpdateValue(std::to_string(stats));
    }
}
