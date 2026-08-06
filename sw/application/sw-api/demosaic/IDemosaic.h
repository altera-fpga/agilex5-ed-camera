/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpDemosaic.h"
#include "IspCommon.h"

#include <cstdint>

namespace SwApi
{

/**
 * High-level interface definition for the VVP Demosaic IP.
 *
 * Usage:
 *   Demosaic instances are configured to a given pattern, one of RGGB, GRBG, GBRG, and BGGR.
 *   You can query this using GetCfaPhase() below.
 *   To set the Demosaic to the desired pattern, use SetCfaPhase() below.
 */
struct IDemosaic
{
    static constexpr auto VVP_DEMOSAIC_PRODUCT_ID = INTEL_VVP_DEMOSAIC_PRODUCT_ID;
    static constexpr auto DemosaicInitialColorFilterArraySetting = TCfaPhase::RGGB;

    static std::shared_ptr<IDemosaic> Create(Hapi::VvpDemosaicPtr spDemosaic,
                                             uint32_t initialOutputWidth, uint32_t initialOutputHeight);
    virtual ~IDemosaic() {};

    virtual bool IsRunning() = 0;

    virtual bool SetBypass(bool to) = 0;
    virtual bool GetBypass() = 0;

    virtual bool SetCfaPhase(TCfaPhase to) = 0;
    virtual TCfaPhase GetCfaPhase() = 0;

    virtual bool SetOutputWidth(uint32_t newWidth) = 0;
    virtual uint32_t GetOutputWidth() = 0;

    virtual bool SetOutputHeight(uint32_t newHeight) = 0;
    virtual uint32_t GetOutputHeight() = 0;

    virtual bool GetFrameStats(uint32_t *stats_out) = 0;
};

}
