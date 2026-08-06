/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "ICsc.h"
#include <vector>
#include <map>

namespace SwApi
{
    namespace Csc
    {

        class CscImplementation : public SwApi::ICsc {
        public:
            CscImplementation(  Hapi::VvpCscPtr spVvpCsc,
                                uint32_t initialOutputWidth, uint32_t initialOutputHeight);

            bool GetLiteMode() override;
            bool GetDebugEnabled() override;
            bool GetIsRunning() override;
            uint8_t GetStatus() override;

            bool SetOutputWidth(uint32_t newWidth) override;
            uint32_t GetOutputWidth() override;
            bool SetOutputHeight(uint32_t newHeight) override;
            uint32_t GetOutputHeight() override;

            uint8_t GetBitsPerSampleIn() override;
            uint8_t GetBitsPerSampleOut() override;
            uint8_t GetCoeffsFracBits() override;
            bool AreCoeffsSigned() override;
            uint8_t GetCoeffsIntBits() override;
            bool AreSummandsSigned() override;
            uint8_t GetSummandsIntBits() override;
            int8_t GetBinaryPointRightMove() override;

            bool GetCoeffs(eIntelVvpCscConversion csConv, intel_vvp_coefficients* coeffs, int* rescale, uint8_t* inCs, uint8_t* outCs) override;
            bool ApplyConversion(eIntelVvpCscConversion csConv) override;
            bool ApplyCoefficientMatrix(const intel_vvp_coefficients* coeffs, int summand_rescale, uint8_t inCs, uint8_t outCs) override;

            int8_t GetInputColorSpace() override;
            int8_t GetOutputColorSpace() override;

            bool CommitWrites() override;
        private:

            Hapi::VvpCscPtr _spVvpCsc;
            int8_t _inputColorSpace;
            int8_t _outputColorSpace;
            uint32_t _outputWidth{0};
            uint32_t _outputHeight{0};
            int _rescale = 0;
            uint8_t _inCs = 0;
            uint8_t _outCs = 0;
            intel_vvp_coefficients _coefficientMatrix;

            eIntelVvpCscConversion _conversionLut[4][4] = {
                {kIntelVvpCscPassthrough, kIntelVvpCscInvalid, kIntelVvpCscComputerSdtv, kIntelVvpCscComputerHdtv},
                {kIntelVvpCscInvalid, kIntelVvpCscPassthrough, kIntelVvpCscStudioSdtv, kIntelVvpCscInvalid},
                {kIntelVvpCscSdtvComputer, kIntelVvpCscSdtvStudio, kIntelVvpCscPassthrough, kIntelVvpCscInvalid},
                {kIntelVvpCscHdtvComputer, kIntelVvpCscInvalid, kIntelVvpCscInvalid, kIntelVvpCscPassthrough}
            };

            std::map<eIntelVvpCscConversion, intel_vvp_coefficients> _coefficientsLut = {
                {kIntelVvpCscInvalid,        CSC_PASSTHROUGH_COEFFS},
                {kIntelVvpCscPassthrough,   CSC_PASSTHROUGH_COEFFS},
                {kIntelVvpCscComputerSdtv,  CSC_RGB_TO_YCCSD_COEFFS},
                {kIntelVvpCscSdtvComputer,  CSC_YCCSD_TO_RGB_COEFFS},
                {kIntelVvpCscStudioSdtv,    CSC_RGBS_TO_YCCSD_COEFFS},
                {kIntelVvpCscSdtvStudio,    CSC_YCCSD_TO_RGBS_COEFFS},
                {kIntelVvpCscComputerHdtv,  CSC_RGB_TO_YCCHD_COEFFS},
                {kIntelVvpCscHdtvComputer,  CSC_YCCHD_TO_RGB_COEFFS}
            };

            std::map<eIntelVvpCscConversion, eIntelVvpCscColorSpace> _inCsLut = {
                {kIntelVvpCscInvalid,        kIntelVvpCsInvalid},
                {kIntelVvpCscPassthrough,   kIntelVvpCsRgb},
                {kIntelVvpCscComputerSdtv,  kIntelVvpCsRgb},
                {kIntelVvpCscSdtvComputer,  kIntelVvpCsYcc},
                {kIntelVvpCscStudioSdtv,    kIntelVvpCsRgb},
                {kIntelVvpCscSdtvStudio,    kIntelVvpCsYcc},
                {kIntelVvpCscComputerHdtv,  kIntelVvpCsRgb},
                {kIntelVvpCscHdtvComputer,  kIntelVvpCsYcc}
            };

            std::map<eIntelVvpCscConversion, eIntelVvpCscColorSpace> _outCsLut = {
                {kIntelVvpCscInvalid,        kIntelVvpCsInvalid},
                {kIntelVvpCscPassthrough,   kIntelVvpCsRgb},
                {kIntelVvpCscComputerSdtv,  kIntelVvpCsYcc},
                {kIntelVvpCscSdtvComputer,  kIntelVvpCsRgb},
                {kIntelVvpCscStudioSdtv,    kIntelVvpCsYcc},
                {kIntelVvpCscSdtvStudio,    kIntelVvpCsRgb},
                {kIntelVvpCscComputerHdtv,  kIntelVvpCsYcc},
                {kIntelVvpCscHdtvComputer,  kIntelVvpCsRgb}
            };

            std::map<eIntelVvpCscConversion, int32_t> _bpsLut = { 
                {kIntelVvpCscInvalid,       8},
                {kIntelVvpCscPassthrough,   8},
                {kIntelVvpCscComputerSdtv,  8},
                {kIntelVvpCscSdtvComputer,  8},
                {kIntelVvpCscStudioSdtv,    8},
                {kIntelVvpCscSdtvStudio,    8},
                {kIntelVvpCscComputerHdtv,  8},
                {kIntelVvpCscHdtvComputer,  10} 
            };
            
        };

    } // namespace Csc
} // namespace SwApi
