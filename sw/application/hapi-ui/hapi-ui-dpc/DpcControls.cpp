/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "DpcControls.h"
#include "DpcUtils.h"
#include "UiElements.h"

#include "IspCommon.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

DpcControls::DpcControls(const std::string& name, 
                         const std::shared_ptr<SwApi::IDpc>& spDpc,
                         bool enableDebugUi)
    : _name(name)
    , _spDpc(spDpc)
    , _enableDebugUi(enableDebugUi)
{
    if (_spDpc) {
        // Initial setup here?
    } else {

    }
}

std::vector<std::shared_ptr<UiControlContainer>> DpcControls::AddUiElements() 
{
    if (not _spDpc) 
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());

    //region Bypass toggle
    auto bypassCB = [this](uint32_t clientID, bool& val) 
    {
        _spDpc->SetBypass(val);
    };
    _spBypassControl = spContainer->AddBoolControl("Bypass", bypassCB, "BypassDPC", false);
    //endregion

    //region CFA Phase
    if (_enableDebugUi) 
    {
        auto cfaCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
        {
            auto chosenCfaType = static_cast<TCfaPhase>(selected._userItemData);
            _spDpc->SetCfaPhase(chosenCfaType);
        };

        std::vector<UiEnumOption> cfaTypeOptions = 
        {
            { "RGGB", static_cast<uint32_t>(TCfaPhase::RGGB) },
            { "GRBG", static_cast<uint32_t>(TCfaPhase::GRBG) },
            { "GBRG", static_cast<uint32_t>(TCfaPhase::GBRG) },
            { "BGGR", static_cast<uint32_t>(TCfaPhase::BGGR) }
        };
        _spCfaPhaseControl = spContainer->AddEnumControl("Bayer pattern type",
                                                        cfaTypeOptions,
                                                        cfaCB,
                                                        "CfaPhase",
                                                        0);
    }
    //endregion

    //region Sensitivity Level
    auto sensitivityCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
    {
        auto chosenSensitivity = (SwApi::DpcSensitivityLevel)selected._userItemData;
        _spDpc->SetSensitivityLevel(chosenSensitivity);
    };

    std::vector<UiEnumOption> sensitivityOptions = 
    {
        { "Very Weak", (uint32_t)DpcSensitivityLevel::VeryWeak },
        { "Weak", (uint32_t)DpcSensitivityLevel::Weak },
        { "Strong", (uint32_t)DpcSensitivityLevel::Strong },
        { "Very Strong", (uint32_t)DpcSensitivityLevel::VeryStrong }
    };
    _spSensitivityControl = spContainer->AddEnumControl("Sensitivity Level",
                                                        sensitivityOptions,
                                                        sensitivityCB,
                                                        "SensitivityLevel",
                                                        0);
    //endregion

    if (_enableDebugUi)
    {
        _spFrameStats = spContainer->AddLabelControl("Checksum:", "");
        _spPixLoCounter = spContainer->AddLabelControl("Pix Lo Counter:", "");
        _spPixHiCounter = spContainer->AddLabelControl("Pix Hi Counter:", "");
    }

    return {std::move(spContainer)};
}

void DpcControls::StatsUpdateLoop()
{
    if (_enableDebugUi)
    {
        uint32_t stats;
        uint32_t lo_count;
        uint32_t hi_count;
        _spDpc->GetFrameStats(&stats, &lo_count, &hi_count);
        _spFrameStats->UpdateValue(std::to_string(stats));
        _spPixLoCounter->UpdateValue(std::to_string(lo_count));
        _spPixHiCounter->UpdateValue(std::to_string(hi_count));
    }
}
