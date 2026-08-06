/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "CrsImplementation.h"
#include "intel_vvp_crs.h"

#include <cmath>

namespace SwApi
{
    std::shared_ptr<ICrs> ICrs::Create( Hapi::VvpCrsPtr spCrs,
                                        uint32_t initialOutputWidth, uint32_t initialOutputHeight) {
        return std::make_shared<Crs::CrsImplementation>(spCrs, initialOutputWidth, initialOutputHeight);
    }

    namespace Crs
    {

        CrsImplementation::CrsImplementation(   Hapi::VvpCrsPtr spVvpCrs,
                                                uint32_t initialOutputWidth, uint32_t initialOutputHeight)
        :   _spVvpCrs(spVvpCrs)
        ,   _inputSampling(kIntelVvpCrsSubsampling444)
        ,   _outputSampling(kIntelVvpCrsSubsampling444) {
            SetOutputWidth(initialOutputWidth);
            SetOutputHeight(initialOutputHeight);
        }

        bool CrsImplementation::GetLiteMode() {
            return intel_vvp_crs_get_lite_mode(_spVvpCrs->GetInstance());
        }

        bool CrsImplementation::GetDebugEnabled() {
            return intel_vvp_crs_get_debug_enabled(_spVvpCrs->GetInstance());
        }

        bool CrsImplementation::GetIsRunning() {
            return intel_vvp_crs_is_running(_spVvpCrs->GetInstance());
        }

        uint8_t CrsImplementation::GetStatus() {
            return intel_vvp_crs_get_status(_spVvpCrs->GetInstance());
        }


        bool CrsImplementation::SetOutputWidth(uint32_t newWidth) {
            auto ret = intel_vvp_core_set_img_info_width(_spVvpCrs->GetInstance(), newWidth);
            bool succeeded = (ret == kIntelVvpCoreOk);
            if (succeeded) {
                _outputWidth = newWidth;
                intel_vvp_crs_commit_writes(_spVvpCrs->GetInstance());
                return true;
            }
            return false;
        }

        uint32_t CrsImplementation::GetOutputWidth() {
            return _outputWidth;
        }

        bool CrsImplementation::SetOutputHeight(uint32_t newHeight) {
            auto ret = intel_vvp_core_set_img_info_height(_spVvpCrs->GetInstance(), newHeight);
            bool succeeded = (ret == kIntelVvpCoreOk);
            if (succeeded) {
                _outputHeight = newHeight;
                intel_vvp_crs_commit_writes(_spVvpCrs->GetInstance());
                return true;
            }
            return false;
        }

        uint32_t CrsImplementation::GetOutputHeight() {
            return _outputHeight;
        }


        bool CrsImplementation::GetIsConversionSupported(eIntelCrsSubsampling input_subsampling, eIntelCrsSubsampling output_subsampling) {
            return intel_vvp_crs_is_conversion_supported(_spVvpCrs->GetInstance(), input_subsampling, output_subsampling);
        }

        eIntelCrsSubsampling CrsImplementation::GetInputSubsampling() {
            return _inputSampling;
        }
        eIntelCrsSubsampling CrsImplementation::GetOutputSubsampling() {
            return _outputSampling;
        }

        bool CrsImplementation::SetInputSubsampling(eIntelCrsSubsampling input_subsampling) {
            if (kIntelVvpCoreOk == intel_vvp_core_set_img_info_subsampling(_spVvpCrs->GetInstance(), input_subsampling)) {
                _inputSampling = input_subsampling;
                intel_vvp_crs_commit_writes(_spVvpCrs->GetInstance());
                return true;
            }
            return false;
        }

        bool CrsImplementation::SetOutputSubsampling(eIntelCrsSubsampling output_subsampling) {
            if (kIntelVvpCoreOk == intel_vvp_crs_set_output_subsampling(_spVvpCrs->GetInstance(), output_subsampling)) {
                _outputSampling = output_subsampling;
                intel_vvp_crs_commit_writes(_spVvpCrs->GetInstance());
                return true;
            }
            return false;
        }

        bool CrsImplementation::CommitWrites() {
            if (kIntelVvpCoreOk == intel_vvp_crs_commit_writes(_spVvpCrs->GetInstance()))
                return true;
            else 
                return false;
        }

        
    } // namespace Crs
} // namespace SwApi