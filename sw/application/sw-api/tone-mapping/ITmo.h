/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "TmoUtils.h"

#include <cstdint>

namespace SwApi {
/**
 * High-level interface definition for the VVP White Balance Correction IP.
 */

struct ITmoBase {
    static std::shared_ptr<ITmoBase> Create(const Hapi::VvpTmoPtr& spTmo,
                                        uint32_t initialOutputWidth, uint32_t initialOutputHeight);

    virtual bool SetOverride(bool override) = 0;
    virtual bool GetOverride() = 0;

    virtual void SetBypass(bool bypass) = 0;
    virtual bool GetBypass() = 0;

    virtual void SetResolution(uint32_t newWidth, uint32_t newHeight) = 0;

    virtual bool SetOutputWidth(uint32_t newWidth) = 0;
    virtual uint32_t GetOutputWidth() = 0;

    virtual bool SetOutputHeight(uint32_t newHeight) = 0;
    virtual uint32_t GetOutputHeight() = 0;

    virtual bool SetRegionOfInterest(intel_vvp_tmo_roi roi) = 0;
    virtual bool SetEnableRoi(bool enable) = 0;
    virtual void SetRoiOutside(bool outside) = 0;
    virtual void SetThreshold(uint32_t threshold) = 0;
    virtual void SetLevel(uint32_t level) = 0;
};

struct ITmo {
    static std::shared_ptr<ITmo> Create(const std::shared_ptr<ITmoBase>& spTmoBase);

    virtual ~ITmo() {};

    /// Enables/disables the core depending on the value of `bypass`, returning
    /// whether the operation succeeded.
    virtual bool SetBypass(bool bypass) = 0;
    virtual bool GetBypass() = 0;

    virtual bool SetOutputWidth(uint32_t newWidth) = 0;
    virtual uint32_t GetOutputWidth() = 0;

    virtual bool SetOutputHeight(uint32_t newHeight) = 0;
    virtual uint32_t GetOutputHeight() = 0;

    virtual bool SetRegionOfInterest(intel_vvp_tmo_roi roi) = 0;
    virtual bool SetEnableRoi(bool enable) = 0;
    virtual bool SetRoiOutside(bool outside) = 0;
    virtual bool SetThreshold(uint32_t threshold) = 0;
    virtual bool SetLevel(uint32_t level) = 0;
};

struct ITmoOverride {
    static std::shared_ptr<ITmoOverride> Create(const std::shared_ptr<ITmoBase>& spTmoBase);

    virtual bool RequestOverride() = 0;
    virtual bool ReleaseOverride() = 0;

    virtual bool SetBypass(bool bypass) = 0;
    virtual bool SetRegionOfInterest(intel_vvp_tmo_roi roi) = 0;
    virtual bool SetEnableRoi(bool enable) = 0;
    virtual bool SetRoiOutside(bool outside) = 0;
    virtual bool SetThreshold(uint32_t threshold) = 0;
    virtual bool SetLevel(uint32_t level) = 0;
};

}
