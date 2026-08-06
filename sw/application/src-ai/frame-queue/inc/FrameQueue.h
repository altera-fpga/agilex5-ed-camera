/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __FrameQueue_H__
#define __FrameQueue_H__

#include "ICoreDLAIntf.h"
#include "IspCommon.h"

#include <cstdint>
#include <lvgl.h>

#include "FrameQueueConfig.h"
#include "SwUtilsMemTransfer.h"

namespace SwApi 
{

    class FrameQueue
    {
        public:
            static std::shared_ptr<FrameQueue> Create(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor);
            FrameQueue(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor);
            virtual ~FrameQueue();
            FrameQueue(const FrameQueue&) = delete;
            FrameQueue(FrameQueue&) = delete;
            FrameQueue& operator=(const FrameQueue&) = delete;

            void Initialise(uint16_t width, uint16_t height);
            void SetVideoResolution(uint16_t width, uint16_t height);

        private:
            static constexpr uintptr_t _cpuAddress = 0x60000000;

            std::shared_ptr<SwApi::ICoreDLAIntf> _spCoreDLAProcessor = nullptr;
    };
} // namespace SwApi

#endif //__FrameQueue_H__

