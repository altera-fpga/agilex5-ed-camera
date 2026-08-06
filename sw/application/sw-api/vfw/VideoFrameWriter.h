/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <cstdint>
#include "HapiVvpVfw.h"
#include "IspCommon.h"
#include "SwUtilsMemTransfer.h"
#include "SwUtilsMemBuffer.h"


namespace SwApi 
{
    using vfw_data_t = SwUtils::mem_buffer_t;

    struct vfw_frame_t
    {
        uint32_t _width;
        uint32_t _height;
        vfw_data_t _data;
    };

    class VideoFrameWriter
    {
        public:
            VideoFrameWriter(Hapi::VvpVfwPtr spVfw, uintptr_t buffer_addr_dma, uintptr_t buffer_addr_fpga);

            void SetResolutionOfImage(uint32_t width, uint32_t height);
            std::pair<uint32_t, uint32_t> GetResolutionOfImage() const;

            void CaptureFrame();
            vfw_frame_t ReadFrame(const uint32_t targetBps, bool RGB);

            bool WaitUntilCommitNoLongerPending();

        private:
            
            Hapi::VvpVfwPtr _spVfw = nullptr;
            uintptr_t _buffer_addr_dma;
            uintptr_t _buffer_addr_fpga;
            std::shared_ptr<SwApi::IMemTransfer> _dataTransfer;
            uint32_t _lineSizeBytes;
            uint32_t _lineStride;
    };
} // namespace SwApi
