/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "DpcImplementation.h"
#include "DpcUtils.h"
#include "intel_vvp_dpc.h"

namespace SwApi 
{

std::shared_ptr<IDpc> IDpc::Create(Hapi::VvpDpcPtr spDpc,
                                   uint32_t initialOutputWidth,
                                   uint32_t initialOutputHeight) 
{
    return std::make_shared<Dpc::DpcImplementation>(spDpc, initialOutputWidth, initialOutputHeight);
}

namespace Dpc 
{

DpcImplementation::DpcImplementation(Hapi::VvpDpcPtr spDpc,
                                     uint32_t initialOutputWidth,
                                     uint32_t initialOutputHeight)
                                     : _spDpc(spDpc)
{
    // Set the output width and height.
    this->SetOutputWidth(initialOutputWidth);
    this->SetOutputHeight(initialOutputHeight);
}

bool DpcImplementation::SetBypass(bool bypass) 
{
    return (intel_vvp_dpc_set_bypass(_spDpc->GetInstance(), bypass) == kIntelVvpCoreOk);
}

bool DpcImplementation::GetBypass() 
{
    return intel_vvp_dpc_get_bypass(_spDpc->GetInstance());
}

bool DpcImplementation::IsRunning() 
{
    return intel_vvp_dpc_is_running(_spDpc->GetInstance());
}

bool DpcImplementation::SetCfaPhase(TCfaPhase to) 
{
    return (intel_vvp_dpc_set_cfa_phase(_spDpc->GetInstance(), static_cast<uint8_t>(to)) == kIntelVvpCoreOk);
}

TCfaPhase DpcImplementation::GetCfaPhase() 
{
    return static_cast<TCfaPhase>(intel_vvp_dpc_get_cfa_phase(_spDpc->GetInstance()));
}

bool DpcImplementation::SetSensitivityLevel(DpcSensitivityLevel to) 
{
    return (intel_vvp_dpc_set_sense_level(_spDpc->GetInstance(), static_cast<uint8_t>(to)) == kIntelVvpCoreOk);
}

DpcSensitivityLevel DpcImplementation::GetSensitivityLevel() 
{
    return (DpcSensitivityLevel)intel_vvp_dpc_get_sense_level(_spDpc->GetInstance());
}

bool DpcImplementation::SetOutputWidth(uint32_t newWidth) 
{
    if (intel_vvp_core_set_img_info_width(_spDpc->GetInstance(), newWidth) == kIntelVvpCoreOk) 
    {
        _outputWidth = newWidth;
        return true;
    }
    return false;
}

uint32_t DpcImplementation::GetOutputWidth() 
{
    return _outputWidth;
}

bool DpcImplementation::SetOutputHeight(uint32_t newHeight) 
{
    if (intel_vvp_core_set_img_info_height(_spDpc->GetInstance(), newHeight) == kIntelVvpCoreOk) 
    {
        _outputHeight = newHeight;
        return true;
    }
    return false;
}

uint32_t DpcImplementation::GetOutputHeight() 
{
    return _outputHeight;
}

bool DpcImplementation::GetFrameStats(uint32_t *stats_out, uint32_t *pix_lo_count, uint32_t *pix_hi_count)
{
    if (!StatsFreezeHandshake()) return false;

    intel_vvp_dpc_get_frame_stats(_spDpc->GetInstance(), stats_out);
    intel_vvp_dpc_get_pix_lo_clip_cnt(_spDpc->GetInstance(), pix_lo_count);
    intel_vvp_dpc_get_pix_hi_clip_cnt(_spDpc->GetInstance(), pix_hi_count);

    intel_vvp_dpc_set_freeze_stats_request(_spDpc->GetInstance(), false);
    
    return true;
}

bool DpcImplementation::StatsFreezeHandshake()
{
    // TODO - replace this with a sub thread rather than stalling the main thread
    intel_vvp_dpc_set_freeze_stats_request(_spDpc->GetInstance(), true);
    usleep(10000); // Wait a 10fps frame, should be a reasonable window.
    for (int i = 0; i < 5; i++)
    {
        if (intel_vvp_dpc_stats_are_frozen(_spDpc->GetInstance()))
        {
            return true;
        }
        usleep(10000);
    }
    std::cout << "Handshake has failed, stats_frozen status bit not set" << std::endl;
    return false;
}

bool DpcImplementation::SetStatsFreeze(bool state)
{
    return intel_vvp_dpc_set_freeze_stats_request(_spDpc->GetInstance(), state) == kIntelVvpCoreOk;
}

} // namespace Dpc

} // namespace SwApi
