/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "LogoControls.h"
#include "UiElements.h"

using namespace SwApi;


LogoControls::LogoControls(const std::shared_ptr<IspMixerHelper>& spMixer, const std::shared_ptr<lvglLogoHelper>& spLogoHelper, bool enableDebugUi)
    : _spMixer(spMixer)
    , _spLogoHelper(spLogoHelper)
    , _enableDebugUi(enableDebugUi)
{
}


std::vector<std::shared_ptr<UiControlContainer>> LogoControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Logo", GetSettingsSectionName());

    if (not _spMixer) 
    {
        spContainer->AddLabelControl("Mixer not present");
        return {std::move(spContainer)};
    }

    auto logoPosCB = [this] (uint32_t clientID, const UiEnumOption& selected, uint32_t)
    {
        _selectedPosition = static_cast<LogoPosition>(selected._userItemData);
        SetLogoPosition(_selectedPosition);
    };

    std::vector<UiEnumOption> logoPositions;
    logoPositions.emplace_back("Disabled", LogoPosition::LogoDisabled);
    logoPositions.emplace_back("Top Left", LogoPosition::TopLeft);
    logoPositions.emplace_back("Top Right", LogoPosition::TopRight);
    logoPositions.emplace_back("Bottom Left", LogoPosition::BottomLeft);
    logoPositions.emplace_back("Bottom Right", LogoPosition::BottomRight);
    logoPositions.emplace_back("Bouncing", LogoPosition::Bouncing);

    _spLogoPosition = spContainer->AddEnumControl("Position",
                                logoPositions,
                                logoPosCB,
                                "logoPositionDropdown",
                                LogoPosition::TopRight);

    auto logoStaticAlphaCB = [this] (uint32_t clientID, float& value)
    {
        if (_spMixer)
            _spMixer->SetLogoOpacity(value);
        if (_spLogoHelper)
            _spLogoHelper->SetLogoOpacity(value);
    };

    spContainer->AddSliderControl("Logo Opacity", 0.0f, 1.0f, logoStaticAlphaCB,
            "MixerLogoStaticAlphaSlider", 0.75f);

    auto screensaverCB = [this] (uint32_t clientID, bool& value)
    {
        if(_fpEnableScreensaver)
            _fpEnableScreensaver(value);
    };

    spContainer->AddBoolControl("Screensaver", false, screensaverCB);

    return {std::move(spContainer)};
}


void LogoControls::SetEnableScreensaverCB(std::function<void(bool)> fpEnableScreensaver)
{
    _fpEnableScreensaver = std::move(fpEnableScreensaver);
}


void LogoControls::SetLogoPosition(const LogoPosition& logoPosition)
{
    _spMixer->SetLogoPosition(logoPosition);
    _isBouncing = ( logoPosition == LogoPosition::Bouncing );
}


void LogoControls::OnScreensaver(bool active)
{
    // When screensaver activated enable logo icon floating
    // otherwise restore position previously selected by user
    _isBouncing = active;
    SetLogoPosition(_isBouncing ? LogoPosition::Bouncing : _selectedPosition);
}