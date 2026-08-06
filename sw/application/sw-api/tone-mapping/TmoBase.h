/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "ITmo.h"
#include "TmoUtils.h"
#include "intel_vvp_core.h"
#include <cstdint>

namespace SwApi {

namespace Tmo {

class TmoBase : public ITmoBase
{
    public:
    TmoBase(const Hapi::VvpTmoPtr& spTmo, uint32_t initialOutputWidth, uint32_t initialOutputHeight);

    bool SetOverride(bool override) override;
    bool GetOverride() override;

    void SetBypass(bool bypass) override;
    bool GetBypass() override;

    void SetResolution(uint32_t newWidth, uint32_t newHeight) override;

    bool SetOutputWidth(uint32_t newWidth) override;
    uint32_t GetOutputWidth() override;

    bool SetOutputHeight(uint32_t newHeight) override;
    uint32_t GetOutputHeight() override;

    bool SetRegionOfInterest(intel_vvp_tmo_roi roi) override;
    bool SetEnableRoi(bool enable) override;
    void SetRoiOutside(bool outside) override;
    void SetThreshold(uint32_t threshold) override;
    void SetLevel(uint32_t level) override;

    private:
    Hapi::VvpTmoPtr _spTmo = nullptr;

    uint32_t _outputWidth{0};
    uint32_t _outputHeight{0};

    bool _override = false;
    TmoStatus _current_status;
    TmoStatus _stored_status;

    void UpdateCore();

};

}

}