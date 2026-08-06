/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "IClipper.h"
#include "ClipperUtils.h"

#include <cstdint>

namespace SwApi {

namespace Clipper {

class ClipperImplementation : public IClipper {
public:
    ClipperImplementation(Hapi::VvpClipperPtr spClipper,
                          uint32_t initialInputHeight=1920, uint32_t initialInputWidth=1080);

    bool SetInputDimensions(uint32_t newWidth, uint32_t newHeight) override;
    uint32_t GetInputWidth() override;
    uint32_t GetInputHeight() override;

    bool IsRunning() override;

    //region Recommended usage - if in doubt, use these.

    /// Switches to Offset clipping, and sets up all registers to suit the requested setting.
    bool SetClipTo(OffsetClipSetting setting) override;
    /// Switches to Rectangle clipping, and sets up all registers to suit the requested setting.
    bool SetClipTo(RectangleClipSetting setting) override;

    //endregion

    ClipperMode GetMode() override;

    //region Setters & Getters for offset clipping mode.
    bool SetClipTopOffset(uint16_t newTopOffset) override;
    bool SetClipLeftOffset(uint16_t newLeftOffset) override;
    bool SetClipBottomOffset(uint16_t newBottomOffset) override;
    bool SetClipRightOffset(uint16_t newRightOffset) override;
    OffsetClipSetting GetOffsetClipSetting() override;
    //endregion

    //region Setters for rectangle clipping mode.
    bool SetRectClipWidth(uint16_t newClipWidth) override;
    bool SetRectClipHeight(uint16_t newClipHeight) override;
    RectangleClipSetting GetRectangleClipSetting() override;
    //endregion
private:
    Hapi::VvpClipperPtr _spClipper = nullptr;
    uint32_t _outputWidth = 1920;
    uint32_t _outputHeight = 1080;
    ClipperMode _mode = ClipperMode::OffsetClipping;
    OffsetClipSetting _offsetClipSetting = OffsetClipSetting{0,0,0,0};
    RectangleClipSetting _rectangleClipSetting = RectangleClipSetting{0,0,1920,1080};
};

} // namespace Clipper

} // namespace SwApi
