/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "ITpg.h"
#include "TpgUtils.h"

#include <cstdint>

namespace SwApi {

namespace Tpg {

class TpgImplementation : public ITpg
{
public:
    TpgImplementation(Hapi::VvpTpgPtr spTpg,
                      uint32_t initialOutputWidth, uint32_t initialOutputHeight,
                      bool isProgressive = true);
    /// Enables/disables the core depending on the value of `enable`, returning
    /// whether the operation succeeded.
    bool SetEnable(bool enable) override;
    bool GetEnable() override;

    uint8_t GetBitsPerSample() override;

    uint8_t GetNumberOfPatternsAvailable() override;
    std::vector<eIntelVvpTpgPatternType> GetAvailablePatternsList() override;
    bool SetSelectedPattern(const intel_vvp_tpg_pattern& pattern) override;
    intel_vvp_tpg_pattern GetLastSelectedPattern() override;

    bool SetOutputWidth(uint32_t newWidth) override;
    uint32_t GetOutputWidth() override;

    bool SetOutputHeight(uint32_t newHeight) override;
    uint32_t GetOutputHeight() override;

    bool SetColorsForUniformPattern(uint16_t color1, uint16_t color2, uint16_t color3) override;
    std::tuple<uint16_t, uint16_t, uint16_t> GetColorsForUniformPattern() override;

    bool SetZonePlateOrigin(uint16_t x_coord, uint16_t y_coord) override;
    std::pair<uint16_t, uint16_t> GetZonePlateOrigin(void) override;
    bool SetZonePlateFactors(uint16_t scaling_factor, uint16_t fine_tune_factor) override;
    std::pair<uint16_t, uint16_t> GetZonePlateFactors(void) override;

    static constexpr uint32_t TpgPreFieldCtrlPacketCount = 0;
    static constexpr uint32_t TpgPostFieldCtrlPacketCount = 0;
private:
    Hapi::VvpTpgPtr _spTpg = nullptr;
    bool _isEnabled = false;
    intel_vvp_tpg_pattern _lastSelectedPattern{kIntelVvpTpgInvalidPattern, kIntelVvpTpgInvalidColor};
    uint32_t _outputWidth{0};
    uint32_t _outputHeight{0};

    std::map<eIntelVvpTpgPatternType, uint8_t> _availablePatterns;

    /// Colors for the uniform pattern
    uint16_t _color1;
    uint16_t _color2;
    uint16_t _color3;
};

} // namespace Tpg

} // namespace SwApi
