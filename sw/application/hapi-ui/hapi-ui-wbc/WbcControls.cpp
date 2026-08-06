/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "WbcControls.h"
#include "IspCommon.h"
#include "UiElements.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

WbcControls::WbcControls(const std::string& name, 
                         const std::shared_ptr<SwApi::Wbc>& spWbc,
                         bool enableDebugUi)
    : _name(name)
    , _spWbc(spWbc)
    , _enableDebugUi(enableDebugUi)
{
}

void WbcControls::BlockUIBasedOnAWBCallback(bool isAWBDrivingWbc)
{
    // Make sure the UI is actually initialised before we start fiddling with it!
    if (_spBypassControl)
    {
        if (isAWBDrivingWbc && _spWbc->GetBypass())
        {
            _spBypassControl->UpdateValue(false, true);
        }

        _spBypassControl->Enable(!isAWBDrivingWbc);

        if (_enableDebugUi)
        {
            _spCfaPhaseControl->Enable(!isAWBDrivingWbc);
        }

        if (isAWBDrivingWbc)
        {
            UpdateControlsFromHardware();
        }

        _spScalar00->Enable(!isAWBDrivingWbc);
        _spScalar01->Enable(!isAWBDrivingWbc);
        _spScalar10->Enable(!isAWBDrivingWbc);
        _spScalar11->Enable(!isAWBDrivingWbc);
    }
}

void WbcControls::UpdateControlsFromHardware()
{
    if(_spScalar00 && _spScalar01 && _spScalar10 && _spScalar11)
    {
        _spScalar00->UpdateValue(_spWbc->GetColorScalerByIdx(0));
        _spScalar01->UpdateValue(_spWbc->GetColorScalerByIdx(1));
        _spScalar10->UpdateValue(_spWbc->GetColorScalerByIdx(2));
        _spScalar11->UpdateValue(_spWbc->GetColorScalerByIdx(3));
    }
}

void WbcControls::BypassBasedOnAWBCallback(bool isBypass)
{
    // Make sure the UI is actually initialised before we start fiddling with it!
    if (_spBypassControl)
    {
        _spBypassControl->UpdateValue(isBypass, true);
    }
}

std::vector<std::shared_ptr<UiControlContainer>> WbcControls::AddUiElements() {
    if (not _spWbc) {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());

    //region Bypass toggle button
    auto bypassCB = [this](uint32_t clientID, bool& val) {
        _spWbc->SetBypass(val);
    };
    _spBypassControl = spContainer->AddBoolControl("Bypass",
                                                   bypassCB,
                                                   "BypassWBC",
                                                   _spWbc->GetBypass());
    //endregion

    if (_enableDebugUi)
    {
        //region CFA Phase
        auto cfaCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void {
            auto chosenCfaType = static_cast<TCfaPhase>(selected._userItemData);
            _spWbc->SetCfaPhase(chosenCfaType);
        };

        std::vector<UiEnumOption> cfaTypeOptions = {
            { "RGGB", static_cast<uint32_t>(TCfaPhase::RGGB) },
            { "GRBG", static_cast<uint32_t>(TCfaPhase::GRBG) },
            { "GBRG", static_cast<uint32_t>(TCfaPhase::GBRG) },
            { "BGGR", static_cast<uint32_t>(TCfaPhase::BGGR) }
        };
        _spCfaPhaseControl = spContainer->AddEnumControl("Bayer pattern type",
                                                        cfaTypeOptions,
                                                        cfaCB,
                                                        "CfaPhase",
                                                        static_cast<uint32_t>(_spWbc->GetCfaPhase()));
    }
    //endregion

    {

        //region Color Scaler
        auto setColorScalerCb = [this](uint8_t idx) {
            return [this, idx](uint32_t clientId, uint32_t& value) {
                _spWbc->SetColorScalerByIdx(idx, value);
            };
        };
        _spScalar00 = spContainer->AddUIntegerControl("Color Scaler 00",
                                                      2048, // Default
                                                      0u, (1 << 19) - 1, // Min, Max
                                                      setColorScalerCb(0b00),
                                                      false);
        _spScalar01 = spContainer->AddUIntegerControl("Color Scaler 01",
                                                      2048, // Default
                                                      0u, (1 << 19) - 1, // Min, Max
                                                      setColorScalerCb(0b01),
                                                      false);
        _spScalar10 = spContainer->AddUIntegerControl("Color Scaler 10",
                                                      2048, // Default
                                                      0u, (1 << 19) - 1, // Min, Max
                                                      setColorScalerCb(0b10),
                                                      false);
        _spScalar11 = spContainer->AddUIntegerControl("Color Scaler 11",
                                                      2048, // Default
                                                      0u, (1 << 19) - 1, // Min, Max
                                                      setColorScalerCb(0b11),
                                                      false);
        //endregion
    }

    auto resetCB = [this] (uint32_t clientID)
    {
        if (_spScalar00 && _spScalar00->GetEnabled())
        {
            _spScalar00->UpdateValue(2048);
            _spScalar01->UpdateValue(2048);
            _spScalar10->UpdateValue(2048);
            _spScalar11->UpdateValue(2048);

            uint32_t scalars[4] = {2048, 2048, 2048, 2048};
            _spWbc->SetAllColorScalers(scalars, UpdatePolicy::Async());
        }
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetCB, "Reset", "OJL/Images/Reset.png" }
    });

    return {std::move(spContainer)};
}
