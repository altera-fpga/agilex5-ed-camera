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
#include "IspMixerHelper.h"
#include "lvglLogoHelper.h"


class LogoControls : public IpUiControls
{
public:
    LogoControls(const std::shared_ptr<IspMixerHelper>& spMixer, const std::shared_ptr<lvglLogoHelper>& spLogoHelper, bool enableDebugUi = true);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "Logo"; };

    void SetEnableScreensaverCB(std::function<void(bool)> fpEnableScreensaver);
    
    void OnScreensaver(bool active);

private:

    using LogoPosition = IspMixerHelper::LogoPosition;

    void SetLogoPosition(const LogoPosition& logoPosition);

    std::shared_ptr<IspMixerHelper> _spMixer;
    std::shared_ptr<lvglLogoHelper> _spLogoHelper;

    std::shared_ptr<UiControlItemEnum> _spLogoPosition;

    bool _enableDebugUi = false;

    LogoPosition _selectedPosition = LogoPosition::TopRight;

    bool _isBouncing = false;
    float _bounceX = 0.0;
    float _bounceY = 0.0;

    bool _bounceXDir = false;
    bool _bounceYDir = false;
    float _bounceXItr = 0.0026;
    float _bounceYItr = 0.0021;

    std::function<void(bool)> _fpEnableScreensaver;
};
