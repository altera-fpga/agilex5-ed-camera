/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __FrameQueueConfig_H__
#define __FrameQueueConfig_H__

#include <cstdint>

namespace SwApi 
{

    namespace FQ
    {
        static constexpr uint32_t FULL_RES_BPS = 10;
        static constexpr uint32_t MINI_BPS = 8;
        static constexpr uint32_t CORE_DLA_BUFFERS = 4;
        static constexpr bool CORE_DLA_BYPASS_OUTPUT_LAYOUT_TRANSFORM = true;

        static constexpr uint32_t INPUT_VIDEO_RESOLUTION_WIDTH = 3840;
        static constexpr uint32_t INPUT_VIDEO_RESOLUTION_HEIGHT = 2160;

        static constexpr uint32_t OVERLAY_WIDTH = 960;
        static constexpr uint32_t OVERLAY_HEIGHT = 540;
        static constexpr uint32_t OVERLAY_BYTES_PER_PIXEL = 1;
        static constexpr uint32_t OVERLAY_BPS = 8;
        static constexpr uint32_t OVERLAY_NUM_BUFFERS = 3;

        static constexpr uint32_t DEFAULT_ALIGNMENT = 0x1000;

        static constexpr uint32_t FRAME_DELAY = 4;

        static constexpr uint32_t inputQueueStartAddress = 0x60000000;
        static constexpr uint32_t inputQueueBufferOffset = 0x03000000;
        static constexpr uint32_t inputQueueScaledStartAddress = 0x18000000;
        static constexpr uint32_t overlayStartAddress = 0x10000000;
    };

} // namespace SwApi

#endif //__FrameQueueConfig_H__

