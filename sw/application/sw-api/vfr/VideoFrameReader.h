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
#include "HapiVvpVfr.h"
#include "IspCommon.h"
#include "SwUtilsMemTransfer.h"
#include "SwUtilsMemBuffer.h"


namespace SwApi 
{
    using vfr_data_t = SwUtils::mem_buffer_t;

    struct vfr_frame_t
    {
        uint32_t _width;
        uint32_t _height;
        uint32_t _bps;
        bool _rgb;
        vfr_data_t _data;
    };

    class VideoFrameReader
    {
        public:
            VideoFrameReader(Hapi::VvpVfrPtr spVfr, uintptr_t buffer_addr_cpu, uintptr_t buffer_addr_fpga);

            void SetResolutionOfImage(uint32_t width, uint32_t height);
            std::pair<uint32_t, uint32_t> GetResolutionOfImage() const;

            void Run(bool state);
            void WriteFrame(vfr_frame_t& frame, bool RGB = true);

        private:
            
            Hapi::VvpVfrPtr _spVfr = nullptr;
            uintptr_t _buffer_addr_fpga;
            std::shared_ptr<SwApi::IMemTransfer> _dataTransfer;
            uint32_t _lineSizeBytes;
            uint32_t _lineStride;
    };
} // namespace SwApi
