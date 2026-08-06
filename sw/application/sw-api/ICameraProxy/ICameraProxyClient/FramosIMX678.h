/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "ICamera.h"

namespace SwApi
{
    class FramosImx678 : public ICamera
    {
    public:
        static std::shared_ptr<ICamera> Create(const uint32_t idx, const uint32_t targetFrameRate);
    };
}