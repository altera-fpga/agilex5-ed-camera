/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CscImplementation.h"
#include "intel_vvp_csc.h"

#include <cmath>

namespace SwApi
{
    std::shared_ptr<ICsc> ICsc::Create(Hapi::VvpCscPtr spCsc,
                                        uint32_t initialOutputWidth, uint32_t initialOutputHeight) 
    {
        return std::make_shared<Csc::CscImplementation>(spCsc, initialOutputWidth, initialOutputHeight);
    }

    namespace Csc
    {
        Csc::CscImplementation::CscImplementation(  Hapi::VvpCscPtr spVvpCsc,
                                                    uint32_t initialOutputWidth, uint32_t initialOutputHeight)
        :   _spVvpCsc(spVvpCsc)
        {
            SetOutputWidth(initialOutputWidth);
            SetOutputHeight(initialOutputHeight);

            _coefficientMatrix = _coefficientsLut[kIntelVvpCscPassthrough];
            _rescale = 0;
            _inCs = 0;
            _outCs = 0;
            intel_vvp_core_set_img_info_colorspace(_spVvpCsc->GetInstance(), kIntelVvpCsRgb);
            if (!intel_vvp_csc_get_lite_mode(_spVvpCsc->GetInstance())) {
                intel_vvp_csc_set_output_color_space(_spVvpCsc->GetInstance(), kIntelVvpCsRgb);
            }
            ApplyConversion(kIntelVvpCscPassthrough);

            // And commit!
            intel_vvp_csc_commit_writes(_spVvpCsc->GetInstance());
        }

        bool CscImplementation::GetLiteMode() {
            return intel_vvp_csc_get_lite_mode(_spVvpCsc->GetInstance());
        }

        bool CscImplementation::GetDebugEnabled() {
            return intel_vvp_csc_get_debug_enabled(_spVvpCsc->GetInstance());
        }
        bool CscImplementation::GetIsRunning() {
            return intel_vvp_csc_is_running(_spVvpCsc->GetInstance());
        }
        uint8_t CscImplementation::GetStatus() {
            return intel_vvp_csc_get_status(_spVvpCsc->GetInstance());
        }

        bool CscImplementation::SetOutputWidth(uint32_t newWidth) {
            auto ret = intel_vvp_core_set_img_info_width(_spVvpCsc->GetInstance(), newWidth);
            bool succeeded = (ret == kIntelVvpCoreOk);
            if (succeeded) {
                _outputWidth = newWidth;
                // And commit!
                intel_vvp_csc_commit_writes(_spVvpCsc->GetInstance());
                return true;
            } else {
                return false;
            }
        }

        uint32_t CscImplementation::GetOutputWidth() {
            return _outputWidth;
        }

        bool CscImplementation::SetOutputHeight(uint32_t newHeight) {
            auto ret = intel_vvp_core_set_img_info_height(_spVvpCsc->GetInstance(), newHeight);
            bool succeeded = (ret == kIntelVvpCoreOk);
            if (succeeded) {
                _outputHeight = newHeight;
                // And commit!
                intel_vvp_csc_commit_writes(_spVvpCsc->GetInstance());                
                return true;
            } else {
                return false;
            }
        }

        uint32_t CscImplementation::GetOutputHeight() {
            return _outputHeight;
        }

        uint8_t CscImplementation::GetBitsPerSampleIn() {
            return intel_vvp_csc_get_bits_per_sample_in(_spVvpCsc->GetInstance());
        }
        uint8_t CscImplementation::GetBitsPerSampleOut() {
            return intel_vvp_csc_get_bits_per_sample_out(_spVvpCsc->GetInstance());
        }
        uint8_t CscImplementation::GetCoeffsFracBits() {
            return intel_vvp_csc_get_coeffs_frac_bits(_spVvpCsc->GetInstance());
        }
        bool CscImplementation::AreCoeffsSigned() {
            return intel_vvp_csc_are_coeffs_signed(_spVvpCsc->GetInstance());
        }
        uint8_t CscImplementation::GetCoeffsIntBits() {
            return intel_vvp_csc_get_coeffs_int_bits(_spVvpCsc->GetInstance());
        }
        bool CscImplementation::AreSummandsSigned() {
            return intel_vvp_csc_are_summands_signed(_spVvpCsc->GetInstance());
        }
        uint8_t CscImplementation::GetSummandsIntBits() {
            return intel_vvp_csc_get_summands_int_bits(_spVvpCsc->GetInstance());
        }
        int8_t CscImplementation::GetBinaryPointRightMove() {
            return intel_vvp_csc_get_binary_point_right_move(_spVvpCsc->GetInstance());
        }

        bool CscImplementation::GetCoeffs(eIntelVvpCscConversion csConv, intel_vvp_coefficients* coeffs, int* rescale, uint8_t* inCs, uint8_t* outCs) {
            if (csConv == kIntelVvpCscInvalid) {
                return false;
            }
            uint8_t bpsIn = intel_vvp_csc_get_bits_per_sample_in(_spVvpCsc->GetInstance());
            *rescale = bpsIn - _bpsLut[csConv];
            *coeffs = _coefficientsLut[csConv];
            *inCs = _inCsLut[csConv];
            *outCs = _outCsLut[csConv];
            return true;
        }

        bool CscImplementation::ApplyConversion(eIntelVvpCscConversion csConv) {
            if (!GetCoeffs(csConv, &_coefficientMatrix, &_rescale, &_inCs, &_outCs)) {
                return false;
            }
            return ApplyCoefficientMatrix(&_coefficientMatrix, _rescale, _inCs, _outCs);
        }


        bool CscImplementation::ApplyCoefficientMatrix(const intel_vvp_coefficients* coeffs, int summand_rescale, uint8_t inCs, uint8_t outCs) {
            _coefficientMatrix = *coeffs;
            _rescale = summand_rescale;
            int err = intel_vvp_csc_set_coeff_data(_spVvpCsc->GetInstance(), &_coefficientMatrix, _rescale);
            if (err != kIntelVvpCoreOk) {
                return false;
            }
            intel_vvp_core_set_img_info_colorspace(_spVvpCsc->GetInstance(), inCs);
            if (!intel_vvp_csc_get_lite_mode(_spVvpCsc->GetInstance())) {
                intel_vvp_csc_set_output_color_space(_spVvpCsc->GetInstance(), outCs);
            }
            // And commit!
            intel_vvp_csc_commit_writes(_spVvpCsc->GetInstance());                
            return true;
        }

        int8_t CscImplementation::GetInputColorSpace() {
            return intel_vvp_core_get_img_info_colorspace(_spVvpCsc->GetInstance());
        }
        int8_t CscImplementation::GetOutputColorSpace() {
            return intel_vvp_csc_get_output_color_space(_spVvpCsc->GetInstance());
        }

        bool CscImplementation::CommitWrites() {
            if (kIntelVvpCoreOk == intel_vvp_csc_commit_writes(_spVvpCsc->GetInstance()))
                return true;
            else 
                return false;
        }

    } // namespace Csc
} // namespace SwApi
