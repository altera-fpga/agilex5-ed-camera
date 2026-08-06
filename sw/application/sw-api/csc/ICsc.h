/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpCsc.h"

typedef enum {
    kIntelVvpCscInvalid = -1,    // Invalid
    kIntelVvpCscPassthrough,    // Passthrough
    kIntelVvpCscComputerSdtv,   // 8 bits (computer) BGR -> CbYCr SDTV
    kIntelVvpCscSdtvComputer,   // 8 bits CbYCr SDTV -> (computer) BGR
    kIntelVvpCscStudioSdtv,     // 8 bits (studio) BGR <-> CbYCr SDTV
    kIntelVvpCscSdtvStudio,     // 8 bits CbYCr SDTV -> (studio) BGR
    kIntelVvpCscComputerHdtv,   // 8 bits (computer) BGR -> CbYCr HDTV, beware that summands are set for a 8 bits per sample input
    kIntelVvpCscHdtvComputer    // 10 bit CbYCr HDTV -> (computer) BGR, beware that summands are set for a 10 bits per sample input
} eIntelVvpCscConversion;

namespace SwApi
{
    class ICsc
    {
    public:
        static std::shared_ptr<ICsc> Create(Hapi::VvpCscPtr spCsc,
                                            uint32_t initialOutputWidth, uint32_t initialOutputHeight); 
        virtual ~ICsc() {}

        virtual bool GetLiteMode() = 0;
        virtual bool GetDebugEnabled() = 0;
        virtual bool GetIsRunning() = 0;
        virtual uint8_t GetStatus() = 0;

        virtual bool SetOutputWidth(uint32_t newWidth) = 0;
        virtual uint32_t GetOutputWidth() = 0;
        virtual bool SetOutputHeight(uint32_t newHeight) = 0;
        virtual uint32_t GetOutputHeight() = 0;

        virtual uint8_t GetBitsPerSampleIn() = 0;
        virtual uint8_t GetBitsPerSampleOut() = 0;
        virtual uint8_t GetCoeffsFracBits() = 0;
        virtual bool AreCoeffsSigned() = 0;
        virtual uint8_t GetCoeffsIntBits() = 0;
        virtual bool AreSummandsSigned() = 0;
        virtual uint8_t GetSummandsIntBits() = 0;
        virtual int8_t GetBinaryPointRightMove() = 0;

        virtual bool GetCoeffs(eIntelVvpCscConversion csConv, intel_vvp_coefficients* coeffs, int* rescale, uint8_t* inCs, uint8_t* outCs) = 0;
        virtual bool ApplyConversion(eIntelVvpCscConversion csConv) = 0;
        virtual bool ApplyCoefficientMatrix(const intel_vvp_coefficients* coeffs, int summand_rescale, uint8_t inCs, uint8_t outCs) = 0;

        virtual int8_t GetInputColorSpace() = 0;
        virtual int8_t GetOutputColorSpace() = 0;

        virtual bool CommitWrites() = 0;
    };
} // namespace SwApi
