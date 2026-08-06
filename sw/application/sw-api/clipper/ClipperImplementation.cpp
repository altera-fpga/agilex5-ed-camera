/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "ClipperImplementation.h"
#include "ClipperUtils.h"
#include "intel_vvp_clipper.h"

namespace SwApi {

std::shared_ptr<IClipper> IClipper::Create(Hapi::VvpClipperPtr spClipper,
                                           uint32_t initialOutputHeight,
                                           uint32_t initialOutputWidth) {
    return std::make_shared<Clipper::ClipperImplementation>(spClipper,
                                                            initialOutputHeight,
                                                            initialOutputWidth);
}

namespace Clipper {

ClipperImplementation::ClipperImplementation(Hapi::VvpClipperPtr spClipper,
                                             uint32_t initialOutputHeight,
                                             uint32_t initialOutputWidth)
                                     : _spClipper(spClipper)
{
    // Set the output width and height.
    SetInputDimensions(initialOutputWidth, initialOutputHeight);

    // Set the Img Info Registers.
    intel_vvp_core_set_img_info_interlace(_spClipper->GetInstance(), 0);
    intel_vvp_core_set_img_info_bps_code(_spClipper->GetInstance(), 0, 8);
    intel_vvp_core_set_img_info_colorspace(_spClipper->GetInstance(), 3);
    intel_vvp_core_set_img_info_subsampling(_spClipper->GetInstance(), 3);

    // Set a default clip setting based on our clipping mode.
    switch (GetMode()) {
        case ClipperMode::OffsetClipping: {
            this->SetClipTo(IClipper::ClipperInitialOffsetClipSetting);
            break;
        }
        case ClipperMode::RectangleClipping: {
            this->SetClipTo(IClipper::ClipperInitialRectangleClipSetting);
            break;
        }
        case ClipperMode::InvalidClipping:
        default: {
            break;
        }
    }

    // And commit!
    intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
}

bool ClipperImplementation::SetInputDimensions(uint32_t newWidth, uint32_t newHeight) {
    auto ret_width = intel_vvp_core_set_img_info_width(_spClipper->GetInstance(), newWidth);
    auto ret_height = intel_vvp_core_set_img_info_height(_spClipper->GetInstance(), newHeight);
    bool succeeded = (ret_width == kIntelVvpCoreOk) && (ret_height == kIntelVvpCoreOk);
    if (succeeded) {
        _outputWidth = newWidth;
        _outputHeight = newHeight;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }

}

uint32_t ClipperImplementation::GetInputWidth() {
    return _outputWidth;
}

uint32_t ClipperImplementation::GetInputHeight() {
    return _outputHeight;
}

bool ClipperImplementation::IsRunning() {
    return intel_vvp_clipper_is_running(_spClipper->GetInstance());
}

bool ClipperImplementation::SetClipTo(OffsetClipSetting setting) {
    auto ret = intel_vvp_clipper_set_clip_offsets(_spClipper->GetInstance(),
                                                  setting.left,
                                                  setting.top,
                                                  setting.right,
                                                  setting.bottom);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_offsetClipSetting = setting;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}

bool ClipperImplementation::SetClipTo(RectangleClipSetting setting) {
    auto ret = intel_vvp_clipper_set_clip_area(_spClipper->GetInstance(),
                                               setting.leftOffset,
                                               setting.topOffset,
                                               setting.clipWidth,
                                               setting.clipHeight);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_rectangleClipSetting = setting;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}

ClipperMode ClipperImplementation::GetMode() {
    return (ClipperMode)intel_vvp_clipper_get_clipping_mode(_spClipper->GetInstance());
}

bool ClipperImplementation::SetClipTopOffset(uint16_t newTopOffset) {
    auto ret = intel_vvp_clipper_set_top_offset(_spClipper->GetInstance(),
                                                newTopOffset);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_offsetClipSetting.top = newTopOffset;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}

bool ClipperImplementation::SetClipLeftOffset(uint16_t newLeftOffset) {
    auto ret = intel_vvp_clipper_set_left_offset(_spClipper->GetInstance(),
                                                  newLeftOffset);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_offsetClipSetting.left = newLeftOffset;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}
bool ClipperImplementation::SetClipBottomOffset(uint16_t newBottomOffset) {
    auto ret = intel_vvp_clipper_set_bottom_offset(_spClipper->GetInstance(),
                                                   newBottomOffset);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_offsetClipSetting.bottom = newBottomOffset;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}
bool ClipperImplementation::SetClipRightOffset(uint16_t newRightOffset) {
    auto ret = intel_vvp_clipper_set_right_offset(_spClipper->GetInstance(),
                                                  newRightOffset);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_offsetClipSetting.right = newRightOffset;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}

OffsetClipSetting ClipperImplementation::GetOffsetClipSetting() {
    return _offsetClipSetting;
}

bool ClipperImplementation::SetRectClipWidth(uint16_t newClipWidth) {
    auto ret = intel_vvp_clipper_set_clip_width(_spClipper->GetInstance(),
                                                newClipWidth);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_rectangleClipSetting.clipWidth = newClipWidth;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}
bool ClipperImplementation::SetRectClipHeight(uint16_t newClipHeight) {
    auto ret = intel_vvp_clipper_set_clip_height(_spClipper->GetInstance(),
                                                newClipHeight);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        this->_rectangleClipSetting.clipHeight = newClipHeight;
        intel_vvp_clipper_commit_writes(_spClipper->GetInstance());
        return true;
    } else {
        return false;
    }
}
RectangleClipSetting ClipperImplementation::GetRectangleClipSetting() {
    return _rectangleClipSetting;
}

} // namespace Clipper

} // namespace SwApi
