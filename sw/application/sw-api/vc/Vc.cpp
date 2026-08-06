/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Vc.h"
#include "intel_vvp_core.h"
#include "intel_vvp_vc.h"
#include <iostream>

#define VC_INST this->_spVc->GetInstance()

namespace SwApi
{

std::shared_ptr<Vc> Vc::Create(Hapi::VvpVcPtr spVc,
                                 uint32_t initialOutputWidth,
                                 uint32_t initialOutputHeight)
{
    return std::make_shared<Vc>(spVc, initialOutputWidth, initialOutputHeight);
}

Vc::Vc(Hapi::VvpVcPtr spVc, uint32_t initialOutputWidth, uint32_t initialOutputHeight):
    VvpCoreBase{"VC"},
    _spVc(spVc),
    _bypass{false},
    _cfaPhase{TCfaPhase::RGGB}
{
    // Set the output width and height.
    SetResolution(initialOutputWidth, initialOutputHeight);
}

bool Vc::SetBypass(bool bypass, UpdatePolicy policy)
{
    auto update_hw = [this, bypass]()->bool {
        return (intel_vvp_vc_set_bypass(_spVc->GetInstance(), bypass) == kIntelVvpCoreOk);
    };

    bool ret = UpdateHw(update_hw, policy);

    if(ret)
        _bypass = bypass;
    else
        std::cerr << "[" << GetName() << "] Failed to change bypass setting\n";

    return ret;
}


bool Vc::GetBypass()
{
    return intel_vvp_vc_get_bypass(VC_INST);
}


uint8_t Vc::GetPip()
{
    return intel_vvp_vc_get_pixels_in_parallel(VC_INST);
}


bool Vc::IsRunning()
{
    return intel_vvp_vc_is_running(VC_INST);
}


bool Vc::HasCfaEnable()
{
    return intel_vvp_vc_get_cfa_enable(VC_INST);
}


bool Vc::SetCfaPhase(TCfaPhase to, UpdatePolicy policy)
{
    auto update_hw = [this, to]()->bool {
        int rv = intel_vvp_vc_set_cfa_phase(VC_INST, static_cast<uint8_t>(to));
        return (rv == kIntelVvpCoreOk || rv == kIntelVvpVcCommitPendingErr);
    };

    bool ret = UpdateHw(update_hw, policy);

    if(ret)
        _cfaPhase = to;
    else
        std::cerr << "[" << GetName() << "] Failed to set CFA phase\n";

    return ret;
}


TCfaPhase Vc::GetCfaPhase()
{
    return static_cast<TCfaPhase>(intel_vvp_vc_get_cfa_phase(VC_INST));
}


bool Vc::SetBlockPixCount(uint16_t blockPixCount, bool autoUpdateRampFrac)
{
    const auto rv = intel_vvp_vc_set_block_pix_count(VC_INST, blockPixCount);
    bool ret = (rv == kIntelVvpCoreOk) || (rv == kIntelVvpVcCommitPendingErr);

    if(ret)
    {
        if (autoUpdateRampFrac)
        {
            // See VC User Guide doc - these are defined as 1 / (blockPixCount +- 1)
            //uint32_t hFracM1 = INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_MSK / (blockPixCount - 1);
            uint32_t hFrac = INTEL_VVP_VC_H_RAMP_FRAC_VALUE_MSK / (blockPixCount);
            uint32_t hFracP1 = INTEL_VVP_VC_H_RAMP_P1_FRAC_VALUE_MSK / (blockPixCount + 1);
            uint32_t hFracM1 = (((INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_MSK / (blockPixCount - 1)) - hFrac) / intel_vvp_vc_get_pixels_in_parallel(VC_INST)) + hFrac - 1;

            const int rv[] = {
                intel_vvp_vc_set_h_ramp_m1_frac(VC_INST, hFracM1),
                intel_vvp_vc_set_h_ramp_frac(VC_INST, hFrac),
                intel_vvp_vc_set_h_ramp_p1_frac(VC_INST, hFracP1)
            };

            ret = ret && (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpVcCommitPendingErr);
            ret = ret && (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpVcCommitPendingErr);
            ret = ret && (rv[2] == kIntelVvpCoreOk || rv[2] == kIntelVvpVcCommitPendingErr);            
        }
    }

    return ret;
}


bool Vc::SetBlockLineCount(uint16_t blockLineCount, bool autoUpdateRampFrac)
{
    const auto rv = intel_vvp_vc_set_block_line_count(VC_INST, blockLineCount);
    bool ret = (rv == kIntelVvpCoreOk) || (rv == kIntelVvpVcCommitPendingErr);

    if(ret)
    {
        if (autoUpdateRampFrac) {
            // See VC User Guide doc - these are defined as 1 / (blockLineCount +- 1)
            uint32_t vFracM1 = INTEL_VVP_VC_V_RAMP_M1_FRAC_VALUE_MSK / (blockLineCount - 1);
            uint32_t vFrac = INTEL_VVP_VC_V_RAMP_FRAC_VALUE_MSK / (blockLineCount);
            uint32_t vFracP1 = INTEL_VVP_VC_V_RAMP_P1_FRAC_VALUE_MSK / (blockLineCount + 1);

            const int rv[] = {
                intel_vvp_vc_set_v_ramp_m1_frac(VC_INST, vFracM1),
                intel_vvp_vc_set_v_ramp_frac(VC_INST, vFrac),
                intel_vvp_vc_set_v_ramp_p1_frac(VC_INST, vFracP1),
            };

            ret = ret && (rv[0] == kIntelVvpCoreOk || rv[0] == kIntelVvpVcCommitPendingErr);
            ret = ret && (rv[1] == kIntelVvpCoreOk || rv[1] == kIntelVvpVcCommitPendingErr);
            ret = ret && (rv[2] == kIntelVvpCoreOk || rv[2] == kIntelVvpVcCommitPendingErr);
        }
    }

    return ret;
}


uint16_t Vc::GetVerticalNumBlocks()
{
    return intel_vvp_vc_get_v_num_blocks(_spVc->GetInstance());
}


uint16_t Vc::GetHorizontalNumBlocks()
{
    return intel_vvp_vc_get_h_num_blocks(_spVc->GetInstance());
}


uint16_t Vc::GetMaxGainMeshPointsAllowed()
{
    return intel_vvp_vc_get_max_gain_mesh_points(VC_INST);
}


bool Vc::GetPerColorGainEnable()
{
    return intel_vvp_vc_get_per_color_gain_enable(VC_INST);
}


bool Vc::SetUpVCForMesh(uint16_t meshXDim, uint16_t meshYDim, UpdatePolicy policy)
{
    auto update_hw = [this, meshXDim, meshYDim]()->bool {
        const uint16_t num_h_blocks = meshXDim - 1;
        const uint16_t num_v_blocks = meshYDim - 1;
        const uint8_t pip = GetPip();

        const int rv[] = {
            intel_vvp_vc_set_h_num_blocks(VC_INST, num_h_blocks),
            intel_vvp_vc_set_v_num_blocks(VC_INST, num_v_blocks)
        };

        bool ret = (rv[0] == kIntelVvpCoreOk) || (rv[0] == kIntelVvpVcCommitPendingErr);
        ret = ret && ((rv[1] == kIntelVvpCoreOk) || (rv[1] == kIntelVvpVcCommitPendingErr));
        ret = ret && SetBlockPixCount(_width / (num_h_blocks * pip), true);
        ret = ret && SetBlockLineCount(_height / num_v_blocks, true);
        return ret;
    };

    bool ret = UpdateHw(update_hw, policy);

    if(ret)
    {
        _meshXDim = meshXDim;
        _meshYDim = meshYDim;
    }
    else
        std::cerr << "[" << GetName() << "] Failed to set configuration for mesh (" << meshXDim << ", " << meshYDim << ")\n";

    return ret;
}


bool Vc::UploadMeshCpLut(const uint32_t cp_idx, const uint32_t* meshCpLutValues)
{
    bool ret = false;

    if (meshCpLutValues && (cp_idx < VC_COLOURPLANES_MAX))
    {
        ret = intel_vvp_vc_update_cp_lut(VC_INST, cp_idx, meshCpLutValues) == kIntelVvpCoreOk;
    }

    return ret;
}


bool Vc::UploadStepLut(const uint32_t* stepLutValues)
{
    bool ret = false;

    if(stepLutValues)
    {
        ret = intel_vvp_vc_update_step_lut(VC_INST, stepLutValues) == kIntelVvpCoreOk;
    }

    return ret;
}


bool Vc::SetResolution(uint32_t width, uint32_t height)
{
    bool ret = false;

    if(intel_vvp_core_set_img_info_width(_spVc->GetInstance(), width) == kIntelVvpCoreOk)
    {
        if(intel_vvp_core_set_img_info_height(_spVc->GetInstance(), height) == kIntelVvpCoreOk)
        {
            _width = width;
            _height = height;
            ret = true;
        }
    }

    return ret;
}


std::pair<uint32_t, uint32_t> Vc::GetResolution()
{
    return {_width, _height};
}


bool Vc::CheckIfResolutionFitsExistingStepMeshConfig(uint16_t newWidth, uint16_t newHeight)
{
    if (!GetHorizontalNumBlocks() || !GetVerticalNumBlocks())
    {
        return true;
    }
    else
    {
        return ((_width % GetHorizontalNumBlocks()) == (newWidth % GetHorizontalNumBlocks())) &&
               ((_height % GetVerticalNumBlocks()) == (newHeight % GetVerticalNumBlocks()));
    }
}


bool Vc::CommitSettings()
{
    return (intel_vvp_vc_commit(_spVc->GetInstance()) == kIntelVvpCoreOk);
}


bool Vc::IsCommitPending()
{
    return intel_vvp_vc_commit_is_pending(_spVc->GetInstance());
}

} // namespace SwApi
