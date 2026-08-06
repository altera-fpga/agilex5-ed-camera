/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpClipper.h"
#include "ClipperUtils.h"

#include <cstdint>

namespace SwApi {

/**
 * High-level interface definition for the VVP Clipper IP.
 *
 * Usage:
 *   Clipper instances are configured to be either in offset mode, or in rectangle mode.
 *   You can query for this using GetMode() below.
 *   Then, use the corresponding set of functions to configure clipping.
 *
 *   For the Offset clip setting, use SetClipTo() or SetClipTopOffset()/SetClipLeftOffset() etc.
 *   For the Rectangle clip setting, use SetClipTo() or SetRectClipWidth()/SetRectClipHeight().
 */
struct IClipper {
    static constexpr auto VVP_CLIPPER_PRODUCT_ID = INTEL_VVP_CLIPPER_PRODUCT_ID;

    /// The setting to which a clipper in Offset Clipping Mode is initialised by Create().
    static constexpr OffsetClipSetting ClipperInitialOffsetClipSetting = {0,0,0,0};
    /// The setting to which a clipper in Rectangle Clipping Mode is initialised by Create().
    static constexpr RectangleClipSetting ClipperInitialRectangleClipSetting = {0,0,1920,1080};

    static std::shared_ptr<IClipper> Create(Hapi::VvpClipperPtr spClipper,
                                            uint32_t initialInputHeight=1920,
                                            uint32_t initialInputWidth=1080);
    virtual ~IClipper() {};

    virtual bool SetInputDimensions(uint32_t newWidth, uint32_t newHeight) = 0;
    virtual uint32_t GetInputWidth() = 0;
    virtual uint32_t GetInputHeight() = 0;

    virtual bool IsRunning() = 0;

    virtual ClipperMode GetMode() = 0;

    //region Recommended usage - if in doubt, use these.
    virtual bool SetClipTo(OffsetClipSetting setting) = 0;
    virtual bool SetClipTo(RectangleClipSetting setting) = 0;
    //endregion

    //region Setters & Getters for offset clipping mode.
    virtual bool SetClipTopOffset(uint16_t newTopOffset) = 0;
    virtual bool SetClipLeftOffset(uint16_t newLeftOffset) = 0;
    virtual bool SetClipBottomOffset(uint16_t newBottomOffset) = 0;
    virtual bool SetClipRightOffset(uint16_t newRightOffset) = 0;
    virtual OffsetClipSetting GetOffsetClipSetting() = 0;
    //endregion

    //region Setters for rectangle clipping mode.
    virtual bool SetRectClipWidth(uint16_t newClipWidth) = 0;
    virtual bool SetRectClipHeight(uint16_t newClipHeight) = 0;
    virtual RectangleClipSetting GetRectangleClipSetting() = 0;
    //endregion
};

}
