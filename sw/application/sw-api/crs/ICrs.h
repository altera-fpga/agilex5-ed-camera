/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpCrs.h"
#include <filesystem>

namespace SwApi
{
    class ICrs
    {
    public:
        static std::shared_ptr<ICrs> Create(Hapi::VvpCrsPtr spCrs,
                                            uint32_t initialOutputWidth, uint32_t initialOutputHeight);
        virtual ~ICrs() {}

        virtual bool GetLiteMode() = 0;
        virtual bool GetDebugEnabled() = 0;
        virtual bool GetIsRunning() = 0;
        virtual uint8_t GetStatus() = 0;

        virtual bool SetOutputWidth(uint32_t newWidth) = 0;
        virtual uint32_t GetOutputWidth() = 0;
        virtual bool SetOutputHeight(uint32_t newHeight) = 0;
        virtual uint32_t GetOutputHeight() = 0;

        virtual bool GetIsConversionSupported(eIntelCrsSubsampling input_subsampling, eIntelCrsSubsampling output_subsampling) = 0;
        virtual eIntelCrsSubsampling GetOutputSubsampling() = 0;
        virtual eIntelCrsSubsampling GetInputSubsampling() = 0;
        virtual bool SetInputSubsampling(eIntelCrsSubsampling input_subsampling) = 0;
        virtual bool SetOutputSubsampling(eIntelCrsSubsampling output_subsampling) = 0;
        virtual bool CommitWrites() = 0;
    };
} // namespace SwApi