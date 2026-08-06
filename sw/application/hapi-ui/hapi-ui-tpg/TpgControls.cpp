/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "TpgControls.h"
#include "TpgUtils.h"
#include "UiElements.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

enum TPGUIPattern
{
    Bar = kIntelVvpTpgBarsPattern,
    Uniform = kIntelVvpTpgUniformPattern,
    ZonePlate = kIntelVvpTpgZonePlatePattern,
    DigitalClock = kIntelVvpTpgDigitalClockPattern,
    Rainbow
};

enum PatternColours
{
    Black = 0,
    Grey,
    White,
    Red,
    Yellow,
    Green,
    Cyan,
    Blue,
    Pink,
    MAX
};

struct Colours
{
    std::string name;
    float blue;
    float green;
    float red;
}; 

static std::vector<Colours> colours = 
{
    {"Black",  0.07, 0.07, 0.07},
    {"Grey",   0.4,  0.4,  0.4},
    {"White",  0.7,  0.7,  0.7},
    {"Red",    0.07, 0.07, 0.7},
    {"Yellow", 0.07, 0.7,  0.7},
    {"Green",  0.07, 0.7,  0.07},
    {"Cyan",   0.7,  0.7,  0.07},
    {"Blue",   0.7,  0.07, 0.07},
    {"Pink",   0.7,  0.07, 0.7}
};

TpgControls::TpgControls(std::shared_ptr<SwApi::ITpg> spTpg, bool canFollowOutput) 
  : _spTpg(spTpg),
    _canFollowOutput(canFollowOutput),
    _bpsScale((1 << _bps) - 1)
{
    if (_spTpg) 
    {
        _bps = spTpg->GetBitsPerSample();
        _bpsScale = (1 << _bps) - 1;
        _spTpg->SetEnable(true);

        _selectedColour = PatternColours::Red;
    }
}

std::vector<std::shared_ptr<UiControlContainer>> TpgControls::AddUiElements() 
{
    if (not _spTpg) 
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>("Test Pattern Generator",
                                                            GetSettingsSectionName());

    auto patternTypeCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) 
    {
        switch (selected._userItemData)
        {
            case TPGUIPattern::Bar:
            {
                _isRainbow = false;
                if (_spColourSelectControl)
                {
                    _spColourSelectControl->Enable(false);
                }

                intel_vvp_tpg_pattern pattern;
                pattern.type = kIntelVvpTpgBarsPattern;
                pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
                _spTpg->SetSelectedPattern(pattern);   
                
                break;
            }
            case TPGUIPattern::ZonePlate:
            {
                _isRainbow = false;
                if (_spColourSelectControl)
                {
                    _spColourSelectControl->Enable(false);
                }

                intel_vvp_tpg_pattern pattern;
                pattern.type = kIntelVvpTpgZonePlatePattern;
                pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
                _spTpg->SetSelectedPattern(pattern);
                _spTpg->SetZonePlateOrigin(_spTpg->GetOutputWidth() / 2, _spTpg->GetOutputHeight() / 2);
                _spTpg->SetZonePlateFactors(6, 1);
                break;
            }
            case TPGUIPattern::Uniform:
            {
                _isRainbow = false;
                if (_spColourSelectControl)
                {
                    _spColourSelectControl->Enable(true);
                }
    
                intel_vvp_tpg_pattern pattern;
                pattern.type = kIntelVvpTpgUniformPattern;
                pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
                _spTpg->SetSelectedPattern(pattern);
                _spTpg->SetColorsForUniformPattern(colours[_selectedColour].blue * _bpsScale,
                                                   colours[_selectedColour].green * _bpsScale,
                                                   colours[_selectedColour].red * _bpsScale);
                break;
            }
            case TPGUIPattern::Rainbow:
            {
                _isRainbow = true;
                _rainbowLowerIdx = PatternColours::Red;
                _rainbowUpperIdx = PatternColours::Red + 1;
                _rainbowItr = 0;
                if (_spColourSelectControl)
                {
                    _spColourSelectControl->Enable(false);
                }

                intel_vvp_tpg_pattern pattern;
                pattern.type = kIntelVvpTpgUniformPattern;
                pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
                _spTpg->SetSelectedPattern(pattern);
                _spTpg->SetColorsForUniformPattern(colours[_rainbowLowerIdx].blue * _bpsScale,
                                                   colours[_rainbowLowerIdx].green * _bpsScale,
                                                   colours[_rainbowLowerIdx].red * _bpsScale);
            }

        }
    };

    bool hasUniform = false;
    bool hasBar = false;
    bool hasZonePlate = false;

    std::vector<eIntelVvpTpgPatternType> patternTypes = _spTpg->GetAvailablePatternsList();

    for (int i = 0; i < (int)patternTypes.size(); i++)
    {
        switch (patternTypes[i])
        {
            case kIntelVvpTpgBarsPattern:
            {
                hasBar = true;
                break;
            }
            case kIntelVvpTpgUniformPattern:
            {
                hasUniform = true;
                break;
            }
            case kIntelVvpTpgZonePlatePattern:
            {
                hasZonePlate = true;
                break;
            }
            default:
                break;
        }
    }


    std::vector<UiEnumOption> patternOptions;

    if (hasBar)
    {
        patternOptions.emplace_back("Bars", TPGUIPattern::Bar);
    }

    if (hasZonePlate)
    {
        patternOptions.emplace_back("Zone Plate", TPGUIPattern::ZonePlate);
    }

    if (hasUniform)
    {
        patternOptions.emplace_back("Uniform", TPGUIPattern::Uniform);
        patternOptions.emplace_back("Rainbow", TPGUIPattern::Rainbow);
    }

    _spPatternSelectControl = spContainer->AddEnumControl("Pattern Select",
                                                          patternOptions,
                                                          patternTypeCB,
                                                          "Pattern",
                                                          TPGUIPattern::Bar);


    auto colourSelectCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) 
    {
        _selectedColour = selected._userItemData;
        _spTpg->SetColorsForUniformPattern(colours[_selectedColour].blue * _bpsScale,
                                           colours[_selectedColour].green * _bpsScale,
                                           colours[_selectedColour].red * _bpsScale);
    };                                                      
    
    std::vector<UiEnumOption> colourOptions;

    if (hasUniform)
    {
        for (int i = 0; i < (int)colours.size(); i++)
        {
            colourOptions.emplace_back(colours[i].name, PatternColours::Black + i);
        }
    }

    _spColourSelectControl = spContainer->AddEnumControl("Color Select",
                                                          colourOptions,
                                                          colourSelectCB,
                                                          "Color",
                                                          0);

    _spColourSelectControl->Enable(false);

    // Resolution select below
    
    auto resSelectCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) 
    {
        _selectedResolution = (TpgResolutions)selected._userItemData;
        switch (_selectedResolution)
        {
            case TpgResolutions::Res1080p:
            {
                _spTpg->SetOutputWidth(1920);
                _spTpg->SetOutputHeight(1080);
                break;
            }
            case TpgResolutions::Res2160p:
            {
                _spTpg->SetOutputWidth(3840);
                _spTpg->SetOutputHeight(2160);
                break;
            }
            case TpgResolutions::FollowOutput:
            {
                _spTpg->SetOutputWidth(_followOutputWidth);
                _spTpg->SetOutputHeight(_followOutputHeight);
                break;
            }
        }
        // Call the registered event handler(s).
        this->ImageSettingsChanged();
    };
    (void)resSelectCB; // unsigned

    std::vector<UiEnumOption> resolutions;

    resolutions.emplace_back("1080p", TpgResolutions::Res1080p);
    resolutions.emplace_back("2160p", TpgResolutions::Res2160p);
    if (_canFollowOutput)
    {
        resolutions.emplace_back("Match HDMI Output", TpgResolutions::FollowOutput);
    }

    return {std::move(spContainer)};
}

void TpgControls::FollowOutputResCallback(uint16_t width, uint16_t height, bool shouldUpdate)
{
    _followOutputWidth = width;
    _followOutputHeight = height;

    // NOTE: shouldUpdate should only be called if the tpg is currently selected, or it may trigger an infinite loop!
    if (_selectedResolution != TpgResolutions::FollowOutput || !shouldUpdate)
    {
        return;
    }
    if (_spTpg->GetOutputWidth() != _followOutputWidth || _spTpg->GetOutputHeight() != _followOutputHeight)
    {
        _spTpg->SetOutputWidth(_followOutputWidth);
        _spTpg->SetOutputHeight(_followOutputHeight);
        this->ImageSettingsChanged();
    }
}

void TpgControls::RainbowUpdateLoop()
{
    if (!_isRainbow)
    {
        return;
    }

    float newBlue = (colours[_rainbowLowerIdx].blue * (1.0 - _rainbowItr)) + (colours[_rainbowUpperIdx].blue * (_rainbowItr));
    float newGreen = (colours[_rainbowLowerIdx].green * (1.0 - _rainbowItr)) + (colours[_rainbowUpperIdx].green * (_rainbowItr));
    float newRed = (colours[_rainbowLowerIdx].red * (1.0 - _rainbowItr)) + (colours[_rainbowUpperIdx].red * (_rainbowItr));

    _spTpg->SetColorsForUniformPattern(newBlue * _bpsScale,
                                       newGreen* _bpsScale,
                                       newRed * _bpsScale);
    _rainbowItr += 0.025f;

    if (_rainbowItr >= 1.0)
    {
        _rainbowLowerIdx = _rainbowUpperIdx;
        if (_rainbowUpperIdx == PatternColours::MAX - 1)
        {
            _rainbowUpperIdx = PatternColours::Red;
        }
        else
        {
            _rainbowUpperIdx += 1;
        }

        _rainbowItr = 0.0;
    }
}