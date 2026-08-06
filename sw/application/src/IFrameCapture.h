/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "VideoFrameWriter.h"

class IFrameCapture
{
public:
    virtual ~IFrameCapture() {}
    virtual SwApi::vfw_frame_t CaptureRawFrame(const uint32_t targetBps, bool RGB) = 0;
    virtual SwApi::vfw_frame_t CaptureProcessedFrame(const uint32_t targetBps, bool RGB) = 0;
};
