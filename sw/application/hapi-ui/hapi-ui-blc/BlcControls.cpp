/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "BlcControls.h"
#include "UiElements.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

BlcControls::BlcControls(const std::string& name,
                         const std::shared_ptr<SwApi::Blc>& spBlc,
                         bool enableDebugUi, bool powerUser)
    : _name(name)
    , _spBlc(spBlc)
    , _enableDebugUi(enableDebugUi)
    , _powerUser(powerUser)
{
    if (_spBlc)
    {
        // Initial setup here?
    }
    else
    {

    }
}

void BlcControls::BlockUIBasedOnBLSCallback(bool isBLSDrivingBLC)
{
    // Make sure the UI is actually initialised before we start fiddling with it!
    if (_spBlackPedestal00)
    {
        bool enable = _powerUser && !isBLSDrivingBLC;

        if (isBLSDrivingBLC && _spBlc->GetBypass())
        {
            _spBypassControl->UpdateValue(false, true);
        }

        if (_enableDebugUi)
        {
            _spCfaPhaseControl->Enable(enable);
        }

        if (isBLSDrivingBLC)
        {
            UpdateControlsFromHardware();
        }

        _spBlackPedestal00->Enable(enable);
        _spBlackPedestal01->Enable(enable);
        _spBlackPedestal10->Enable(enable);
        _spBlackPedestal11->Enable(enable);

        _spColorScalar00->Enable(enable);
        _spColorScalar01->Enable(enable);
        _spColorScalar10->Enable(enable);
        _spColorScalar11->Enable(enable);

        _spBypassControl->Enable(!isBLSDrivingBLC);
    }
}


void BlcControls::UpdateControlsFromHardware()
{
    if(_spBlackPedestal00 && _spBlackPedestal01 && _spBlackPedestal10 && _spBlackPedestal11)
    {
        _spBlackPedestal00->UpdateValue(_spBlc->GetBlackPedestalByIdx(0));
        _spBlackPedestal01->UpdateValue(_spBlc->GetBlackPedestalByIdx(1));
        _spBlackPedestal10->UpdateValue(_spBlc->GetBlackPedestalByIdx(2));
        _spBlackPedestal11->UpdateValue(_spBlc->GetBlackPedestalByIdx(3));
    }

    if(_spColorScalar00 && _spColorScalar01 && _spColorScalar10 && _spColorScalar11)
    {
        _spColorScalar00->UpdateValue(_spBlc->GetColorScalerByIdx(0));
        _spColorScalar01->UpdateValue(_spBlc->GetColorScalerByIdx(1));
        _spColorScalar10->UpdateValue(_spBlc->GetColorScalerByIdx(2));
        _spColorScalar11->UpdateValue(_spBlc->GetColorScalerByIdx(3));
    }
}


void BlcControls::BypassBasedOnAWBCallback(bool isBypass)
{
    // Make sure the UI is actually initialised before we start fiddling with it!
    if (_spBypassControl)
    {
        _spBypassControl->UpdateValue(isBypass, true);
    }
}

std::vector<std::shared_ptr<UiControlContainer>> BlcControls::AddUiElements()
{
    if (not _spBlc)
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());

    //region Bypass toggle button
    auto bypassCB = [this](uint32_t clientID, bool& val)
    {
        _spBlc->SetBypass(val);
    };
    _spBypassControl = spContainer->AddBoolControl("Bypass",
                                                   bypassCB,
                                                   "BypassBLC",
                                                   _spBlc->GetBypass());
    //endregion


    //region CFA Phase
    if (_enableDebugUi)
    {
        auto cfaCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
        {
            auto chosenCfaType = (TCfaPhase)selected._userItemData;
            _spBlc->SetCfaPhase(chosenCfaType);
        };

        std::vector<UiEnumOption> cfaTypeOptions = {
            { "RGGB", (uint32_t)TCfaPhase::RGGB },
            { "GRBG", (uint32_t)TCfaPhase::GRBG },
            { "GBRG", (uint32_t)TCfaPhase::GBRG },
            { "BGGR", (uint32_t)TCfaPhase::BGGR }
        };
        _spCfaPhaseControl = spContainer->AddEnumControl("Bayer pattern type",
                                                        cfaTypeOptions,
                                                        cfaCB,
                                                        "CfaPhase",
                                                        (uint32_t)_spBlc->GetCfaPhase());

        //region Clip to zero toggle
        auto clipZeroCb = [this](uint32_t clientID, bool& val) -> void
        {
            _spBlc->SetClipZero(val);
        };
        _spClipZeroControl = spContainer->AddBoolControl("Clip to zero",
                                                        clipZeroCb,
                                                        "ClipZeroBlc",
                                                        _spBlc->GetClipZero());
    }
    //endregion

    auto setBlackPedestalCb = [this](uint8_t idx)
    {
        return [this, idx](uint32_t clientId, uint32_t& value)
        {
            _spBlc->SetBlackPedestalByIdx(idx, value);
        };
    };
    _spBlackPedestal00 = spContainer->AddUIntegerControl("Black Pedestal 00",
                                                            _spBlc->GetBlackPedestalByIdx(0b00), // Default
                                                            0u, 10000u, // Min, Max
                                                            setBlackPedestalCb(0b00),
                                                            false);
    _spBlackPedestal01 = spContainer->AddUIntegerControl("Black Pedestal 01",
                                                            _spBlc->GetBlackPedestalByIdx(0b01), // Default
                                                            0u, 10000u, // Min, Max
                                                            setBlackPedestalCb(0b01),
                                                            false);
    _spBlackPedestal10 = spContainer->AddUIntegerControl("Black Pedestal 10",
                                                            _spBlc->GetBlackPedestalByIdx(0b10), // Default
                                                            0u, 10000u, // Min, Max
                                                            setBlackPedestalCb(0b10),
                                                            false);
    _spBlackPedestal11 = spContainer->AddUIntegerControl("Black Pedestal 11",
                                                            _spBlc->GetBlackPedestalByIdx(0b11), // Default
                                                            0u, 10000u, // Min, Max
                                                            setBlackPedestalCb(0b11),
                                                            false);

    auto setColorScalerCb = [this](uint8_t idx)
    {
        return [this, idx](uint32_t clientId, uint32_t& value)
        {
            _spBlc->SetColorScalerByIdx(idx, value);
        };
    };
    _spColorScalar00 = spContainer->AddUIntegerControl("Color Scaler 00",
                                                        _spBlc->GetColorScalerByIdx(0b00), // Default
                                                        0u, (1 << 19) - 1, // Min, Max
                                                        setColorScalerCb(0b00),
                                                        false);
    _spColorScalar01 = spContainer->AddUIntegerControl("Color Scaler 01",
                                                        _spBlc->GetColorScalerByIdx(0b01), // Default
                                                        0u, (1 << 19) - 1, // Min, Max
                                                        setColorScalerCb(0b01),
                                                        false);
    _spColorScalar10 = spContainer->AddUIntegerControl("Color Scaler 10",
                                                        _spBlc->GetColorScalerByIdx(0b10), // Default
                                                        0u, (1 << 19) - 1, // Min, Max
                                                        setColorScalerCb(0b10),
                                                        false);
    _spColorScalar11 = spContainer->AddUIntegerControl("Color Scaler 11",
                                                        _spBlc->GetColorScalerByIdx(0b11), // Default
                                                        0u, (1 << 19) - 1, // Min, Max
                                                        setColorScalerCb(0b11),
                                                        false);

    // Only add the reset button in poweruser mode. Typically the whole
    // UI is greyed out so people can't use it
    if (_powerUser)
    {
        auto resetCB = [this] (uint32_t clientID)
        {
            _spBlackPedestal00->UpdateValue(0);
            _spBlackPedestal01->UpdateValue(0);
            _spBlackPedestal10->UpdateValue(0);
            _spBlackPedestal11->UpdateValue(0);

            _spColorScalar00->UpdateValue(0);
            _spColorScalar01->UpdateValue(0);
            _spColorScalar10->UpdateValue(0);
            _spColorScalar11->UpdateValue(0);

            uint32_t values[4] = {0, 0, 0, 0};
            _spBlc->SetPedestalsAndScalers(values, values, UpdatePolicy::Async());

        };

        spContainer->AddHeaderButtons( {
            { "CornerControlsReset", resetCB, "Reset", "OJL/Images/Reset.png" }
        });
    }

    return {std::move(spContainer)};
}
