/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "IDemosaic.h"
#include "IspCommon.h"

#include <cstdint>

namespace SwApi 
{

namespace Demosaic 
{

class DemosaicImplementation : public IDemosaic 
{
public:
    DemosaicImplementation(Hapi::VvpDemosaicPtr spDemosaic,
                           uint32_t initialOutputWidth, uint32_t initialOutputHeight);

    bool IsRunning() override;

    bool SetBypass(bool to) override;
    bool GetBypass() override;

    virtual bool SetCfaPhase(TCfaPhase to) override;
    virtual TCfaPhase GetCfaPhase() override;

    bool SetOutputWidth(uint32_t newWidth) override;
    uint32_t GetOutputWidth() override;

    bool SetOutputHeight(uint32_t newHeight) override;
    uint32_t GetOutputHeight() override;

    bool GetFrameStats(uint32_t *stats_out) override;

private:
    Hapi::VvpDemosaicPtr _spDemosaic = nullptr;

    uint32_t _outputWidth{0};
    uint32_t _outputHeight{0};
};

} // namespace Demosaic

} // namespace SwApi
