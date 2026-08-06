/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "ICrs.h"
#include <vector>
#include <set>

namespace SwApi
{
    namespace Crs
    {
        class CrsImplementation : public SwApi::ICrs
        {
        public:
            CrsImplementation(  Hapi::VvpCrsPtr spVvpCrs,
                                uint32_t initialOutputWidth, uint32_t initialOutputHeight);
            bool GetLiteMode() override;
            bool GetDebugEnabled() override;
            bool GetIsRunning() override;
            uint8_t GetStatus() override;

            bool SetOutputWidth(uint32_t newWidth) override;
            uint32_t GetOutputWidth() override;
            bool SetOutputHeight(uint32_t newHeight) override;
            uint32_t GetOutputHeight() override;

            bool GetIsConversionSupported(eIntelCrsSubsampling input_subsampling, eIntelCrsSubsampling output_subsampling) override;
            eIntelCrsSubsampling GetInputSubsampling() override;
            eIntelCrsSubsampling GetOutputSubsampling() override;
            bool SetInputSubsampling(eIntelCrsSubsampling input_subsampling) override;
            bool SetOutputSubsampling(eIntelCrsSubsampling output_subsampling) override;
            bool CommitWrites() override;

        private:
            Hapi::VvpCrsPtr _spVvpCrs;
            eIntelCrsSubsampling _inputSampling;
            eIntelCrsSubsampling _outputSampling;
            uint32_t _outputWidth{0};
            uint32_t _outputHeight{0};
        };

    } // namespace Crs
} // namespace SwApi