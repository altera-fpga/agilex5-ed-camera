/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "DemosaicImplementation.h"
#include "intel_vvp_demosaic.h"

namespace SwApi {

std::shared_ptr<IDemosaic> IDemosaic::Create(Hapi::VvpDemosaicPtr spDemosaic,
                                             uint32_t initialOutputWidth,
                                             uint32_t initialOutputHeight) 
{
    return std::make_shared<Demosaic::DemosaicImplementation>(spDemosaic,
                                                              initialOutputWidth,
                                                              initialOutputHeight);
}

namespace Demosaic 
{

DemosaicImplementation::DemosaicImplementation(Hapi::VvpDemosaicPtr spDemosaic,
                                               uint32_t initialOutputWidth,
                                               uint32_t initialOutputHeight)
    : _spDemosaic(spDemosaic)
{
    // Set the output width and height.
    this->SetOutputWidth(initialOutputWidth);
    this->SetOutputHeight(initialOutputHeight);

    // Set interlace setting to progressive.
    intel_vvp_core_set_img_info_interlace(_spDemosaic->GetInstance(), 3);

    // Set the CFA pattern to the default.
    this->SetCfaPhase(IDemosaic::DemosaicInitialColorFilterArraySetting);

    // Disable bypass
    this->SetBypass(false);
}

bool DemosaicImplementation::IsRunning() 
{
    return intel_vvp_demosaic_is_running(_spDemosaic->GetInstance());
}

bool DemosaicImplementation::SetBypass(bool to) 
{
    return (intel_vvp_demosaic_set_bypass(_spDemosaic->GetInstance(), to) == kIntelVvpCoreOk);
}

bool DemosaicImplementation::GetBypass() 
{
    return intel_vvp_demosaic_get_bypass(_spDemosaic->GetInstance());
}

bool DemosaicImplementation::SetCfaPhase(TCfaPhase to) 
{
    return (intel_vvp_demosaic_set_cfa_phase(_spDemosaic->GetInstance(), static_cast<uint8_t>(to)) == kIntelVvpCoreOk);
}

TCfaPhase DemosaicImplementation::GetCfaPhase() 
{
    return static_cast<TCfaPhase>(intel_vvp_demosaic_get_cfa_phase(_spDemosaic->GetInstance()));
}

bool DemosaicImplementation::SetOutputWidth(uint32_t newWidth) 
{
    if (intel_vvp_core_set_img_info_width(_spDemosaic->GetInstance(), newWidth) == kIntelVvpCoreOk) 
    {
        _outputWidth = newWidth;
        return true;
    }
    return false;
}

uint32_t DemosaicImplementation::GetOutputWidth() 
{
    return _outputWidth;
}

bool DemosaicImplementation::SetOutputHeight(uint32_t newHeight) 
{
    if (intel_vvp_core_set_img_info_height(_spDemosaic->GetInstance(), newHeight) == kIntelVvpCoreOk)
    {
        _outputHeight = newHeight;
        return true;
    }
    return false;
}

uint32_t DemosaicImplementation::GetOutputHeight() 
{
    return _outputHeight;
}

bool DemosaicImplementation::GetFrameStats(uint32_t *stats_out)
{
    return intel_vvp_demosaic_get_frame_stats(_spDemosaic->GetInstance(), stats_out) == kIntelVvpCoreOk;
}

} // namespace Demosaic

} // namespace SwApi
