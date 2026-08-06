/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "ClipperUtils.h"

#include <iostream>
#include <sstream>

namespace SwApi {

std::string ToString(const ClipperMode& m) {
    std::stringstream ss; ss << m;
    return ss.str();
}

std::string ToString(const OffsetClipSetting& m) {
    std::stringstream ss; ss << m;
    return ss.str();
}

std::string ToString(const RectangleClipSetting& m) {
    std::stringstream ss; ss << m;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const ClipperMode& mode) {
    switch (mode) {
        case ClipperMode::OffsetClipping: {
            return os << "ClipperMode::OffsetClipping";
            break;
        }
        case ClipperMode::RectangleClipping: {
            return os << "ClipperMode::RectangleClipping";
            break;
        }
        default:
        case ClipperMode::InvalidClipping: {
            return os << "ClipperMode::RectangleClipping";
            break;
        }
    }

}

std::ostream& operator<<(std::ostream& os, const SwApi::OffsetClipSetting& setting) {
    return os << "OffsetClipSetting("
        << "top: " << setting.top << ", "
        << "left: " << setting.left << ", "
        << "bottom: " << setting.bottom << ", "
        << "right: " << setting.right
        << ')';
}

std::ostream& operator<<(std::ostream& os, const SwApi::RectangleClipSetting& setting) {
    return os << "RectangleClipSetting("
        << "topOffset: " << setting.topOffset << ", "
        << "leftOffset: " << setting.leftOffset << ", "
        << "width: " << setting.clipWidth << ", "
        << "height: " << setting.clipHeight
        << ')';
}

} // namespace SwApi
