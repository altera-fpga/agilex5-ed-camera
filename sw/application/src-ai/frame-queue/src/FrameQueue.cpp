/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "FrameQueue.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <iomanip>

#include <sys/time.h>
#include <lvgl.h>
#include <src/core/lv_refr_private.h>
#include "lv_labelled_box.h"
#include "lv_skeleton.h"
#include "HapiCoreDLA.h"

namespace SwApi 
{
    std::shared_ptr<FrameQueue> FrameQueue::Create(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor)
    {
        return std::make_shared<FrameQueue>(spCoreDLAProcessor);
    }

    FrameQueue::FrameQueue(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor)
    : _spCoreDLAProcessor(spCoreDLAProcessor)
    {
    }

    FrameQueue::~FrameQueue()
    {
    }

    void FrameQueue::Initialise(uint16_t width, uint16_t height)
    {
        SetVideoResolution(width, height);
    }

    void FrameQueue::SetVideoResolution(uint16_t width, uint16_t height)
    {
        tVideoCoreConfig videoCoreConfig;

        videoCoreConfig.inputQueueStartAddress = SwApi::FQ::inputQueueStartAddress;
        videoCoreConfig.inputQueueBufferOffset = SwApi::FQ::inputQueueBufferOffset;
        videoCoreConfig.inputQueueScaledStartAddress = SwApi::FQ::inputQueueScaledStartAddress;

        videoCoreConfig.FQ_FULL_RES_BPS = SwApi::FQ::FULL_RES_BPS;

        videoCoreConfig.FQ_INPUT_VIDEO_RESOLUTION_WIDTH = width;
        videoCoreConfig.FQ_INPUT_VIDEO_RESOLUTION_HEIGHT = height;
        videoCoreConfig.FQ_OUTPUT_VIDEO_RESOLUTION_WIDTH = width;
        videoCoreConfig.FQ_OUTPUT_VIDEO_RESOLUTION_HEIGHT = height;

        videoCoreConfig.FQ_DEFAULT_ALIGNMENT = SwApi::FQ::DEFAULT_ALIGNMENT;

        videoCoreConfig.FQ_FRAME_DELAY = SwApi::FQ::FRAME_DELAY;

        videoCoreConfig.numInputQueueSlots = SwApi::FQ::FRAME_DELAY + 2;

        _spCoreDLAProcessor->SendUserMessage(UserMessageType::UserMessageType_VideoCoreConfig, &videoCoreConfig, sizeof(videoCoreConfig));
    }
} // namespace SwApi

