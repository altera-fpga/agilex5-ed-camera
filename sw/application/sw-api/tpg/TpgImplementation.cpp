/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "TpgImplementation.h"
#include "intel_vvp_tpg.h"

namespace SwApi {

std::shared_ptr<ITpg> ITpg::Create(Hapi::VvpTpgPtr spTpg,
                                   uint32_t initialOutputWidth,
                                   uint32_t initialOutputHeight)
{
    return std::make_shared<Tpg::TpgImplementation>(spTpg, initialOutputWidth, initialOutputHeight);
}

namespace Tpg {

TpgImplementation::TpgImplementation(Hapi::VvpTpgPtr spTpg,
                                     uint32_t initialOutputWidth,
                                     uint32_t initialOutputHeight,
                                     bool isProgressive)
                                     : _spTpg(spTpg)
                                     , _isEnabled(false)
{
    // Set the output width and height.
    this->SetOutputWidth(initialOutputWidth);
    this->SetOutputHeight(initialOutputHeight);

    // Set interlace setting to progressive.
    intel_vvp_core_set_img_info_interlace(_spTpg->GetInstance(), 3);



    // Instantiate our list of patterns
    {
        for (auto i = 0; i < GetNumberOfPatternsAvailable(); i++)
        {
            intel_vvp_tpg_pattern store;

            intel_vvp_tpg_get_pattern_data(_spTpg->GetInstance(), i, &store);
            
            _availablePatterns.insert({store.type, i});
        }
    }

    // And commit!
    intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
}

bool TpgImplementation::SetEnable(bool enable)
{
    if (intel_vvp_tpg_enable(_spTpg->GetInstance(), enable) == kIntelVvpCoreOk)
    {
        _isEnabled = enable;
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
    }

    return _isEnabled;
}

bool TpgImplementation::GetEnable()
{
    return _isEnabled;
}

uint8_t TpgImplementation::GetBitsPerSample()
{
    return intel_vvp_tpg_get_bits_per_sample(_spTpg->GetInstance());
}

uint8_t TpgImplementation::GetNumberOfPatternsAvailable()
{
    return intel_vvp_tpg_get_num_patterns(_spTpg->GetInstance());
}

std::vector<eIntelVvpTpgPatternType> TpgImplementation::GetAvailablePatternsList()
{
    std::vector<eIntelVvpTpgPatternType> patternTypes;

    for (auto const& it : _availablePatterns)
    {
        patternTypes.push_back(it.first);
    }

    return patternTypes;
}

bool TpgImplementation::SetSelectedPattern(const intel_vvp_tpg_pattern& pattern)
{
    if (intel_vvp_tpg_set_pattern(_spTpg->GetInstance(), _availablePatterns.at(pattern.type)) == kIntelVvpCoreOk)
    {
        _lastSelectedPattern = pattern;
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
        return true;
    }

    return false;
}

intel_vvp_tpg_pattern TpgImplementation::GetLastSelectedPattern()
{
    return _lastSelectedPattern;
}

bool TpgImplementation::SetOutputWidth(uint32_t newWidth)
{
    if (intel_vvp_core_set_img_info_width(_spTpg->GetInstance(), newWidth) == kIntelVvpCoreOk)
    {
        _outputWidth = newWidth;
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
        return true;
    }

    return false;
}

uint32_t TpgImplementation::GetOutputWidth()
{
    return _outputWidth;
}

bool TpgImplementation::SetOutputHeight(uint32_t newHeight)
{
    if (intel_vvp_core_set_img_info_height(_spTpg->GetInstance(), newHeight) == kIntelVvpCoreOk)
    {
        _outputHeight = newHeight;
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
        return true;
    }

    return false;
}

uint32_t TpgImplementation::GetOutputHeight()
{
    return _outputHeight;
}

bool TpgImplementation::SetColorsForUniformPattern(uint16_t color1, uint16_t color2, uint16_t color3)
{
    if (intel_vvp_tpg_set_colors(_spTpg->GetInstance(), color1, color2, color3) == kIntelVvpCoreOk)
    {
        _color1 = color1; _color2 = color2; _color3 = color3;
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
        return true;
    }

    return false;
}

std::tuple<uint16_t, uint16_t, uint16_t> TpgImplementation::GetColorsForUniformPattern()
{
    return {_color1, _color2, _color3};
}

bool TpgImplementation::SetZonePlateOrigin(uint16_t x_coord, uint16_t y_coord)
{
    if (intel_vvp_tpg_set_zone_plate_origin(_spTpg->GetInstance(), x_coord, y_coord) == kIntelVvpCoreOk)
    {
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
        return true;
    }

    return false;
}

std::pair<uint16_t, uint16_t> TpgImplementation::GetZonePlateOrigin(void)
{
    uint16_t x_coord;
    uint16_t y_coord;

    if (intel_vvp_tpg_get_zone_plate_origin(_spTpg->GetInstance(), &x_coord, &y_coord) == kIntelVvpCoreOk)
    {
        return {x_coord, y_coord};
    }

    return {0, 0};
}

bool TpgImplementation::SetZonePlateFactors(uint16_t scaling_factor, uint16_t fine_tune_factor)
{
    if (intel_vvp_tpg_set_zone_plate_factors(_spTpg->GetInstance(), scaling_factor, fine_tune_factor) == kIntelVvpCoreOk)
    {
        intel_vvp_tpg_commit_writes(_spTpg->GetInstance());
        return true;
    }

    return false;
}

std::pair<uint16_t, uint16_t> TpgImplementation::GetZonePlateFactors(void)
{
    uint16_t scaling_factor;
    uint16_t fine_tune_factor;

    if (intel_vvp_tpg_get_zone_plate_origin(_spTpg->GetInstance(), &scaling_factor, &fine_tune_factor) == kIntelVvpCoreOk)
    {
        return {scaling_factor, fine_tune_factor};
    }

    return {0, 0};
}

} // namespace Tpg

} // namespace SwApi
