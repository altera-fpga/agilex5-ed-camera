/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "HapiUiObservable.hpp"
#include "IpUiControls.h"
#include "ITpg.h"

/// Encapsulates UI functionality for the Test Pattern Generator IP
class TpgControls : public IpUiControls,
                    public ObservableIpUi
{
public:

    /// Creates an instance of the class that'll manage the given TPG instance.
    TpgControls(std::shared_ptr<SwApi::ITpg> spTpg, bool canFollowOutput = false);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "TestPatternGenerator"; };

    void FollowOutputResCallback(uint16_t width, uint16_t height, bool shouldUpdate);
    void RainbowUpdateLoop();
private:

    std::shared_ptr<SwApi::ITpg> _spTpg;

    bool _canFollowOutput;

    std::shared_ptr<UiControlItemBoolean> _spEnableControl;
    std::shared_ptr<UiControlItemEnum> _spPatternSelectControl;
    std::shared_ptr<UiControlItemEnum> _spColourSelectControl;

    enum TpgResolutions
    {
        Res1080p = 0,
        Res2160p,
        FollowOutput
    };

    TpgResolutions _selectedResolution = TpgResolutions::Res1080p;

    uint16_t _followOutputWidth = 1920;
    uint16_t _followOutputHeight = 1080;

    uint32_t _selectedColour = 0;

    bool _enableDebugUi = false;

    uint8_t _bps = 10;
    uint32_t _bpsScale;

    bool _isRainbow = false;
    int _rainbowLowerIdx = 0;
    int _rainbowUpperIdx = 0;
    float _rainbowItr = 0.0;
};
