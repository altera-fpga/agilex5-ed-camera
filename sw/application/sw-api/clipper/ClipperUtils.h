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

#include <string>
#include <iosfwd>

namespace SwApi {

/// The two possible choices of clipping mode.
/// In 'OffsetClipping' mode, the clipping is specified as 4 offsets (top, left, bottom, right).
/// (the values for 'bottom' and 'right' are relative to the bottom-right corner of the image.)
///
/// In 'RectangleClipping' mode, the clipping is is specified with a top-left
/// offset and the dimensions of the clipped area.
enum class ClipperMode {
    OffsetClipping = kIntelVvpOffsetClipping,
    RectangleClipping = kIntelVvpRectangleClipping,
    InvalidClipping = kIntelVvpInvalidClipping,
};

std::string ToString(const ClipperMode& m);

/// Stores a valid setting for a clipper in Offset Clipping Mode.
struct OffsetClipSetting {
    uint16_t top;
    uint16_t left;
    uint16_t bottom;
    uint16_t right;
};

std::string ToString(const OffsetClipSetting& m);

/// Stores a valid setting for a clipper in Rectangle Clipping Mode.
struct RectangleClipSetting {
    uint16_t topOffset;
    uint16_t leftOffset;
    uint16_t clipWidth;
    uint16_t clipHeight;
};

std::string ToString(const RectangleClipSetting& m);

std::ostream& operator<<(std::ostream& os, const ClipperMode& mode);
std::ostream& operator<<(std::ostream& os, const SwApi::OffsetClipSetting& setting);
std::ostream& operator<<(std::ostream& os, const SwApi::RectangleClipSetting& setting);

} // namespace SwApi

