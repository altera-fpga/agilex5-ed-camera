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

#include <cstdint>

namespace SwApi {

namespace Tmo {

class TmoOverride : public ITmoOverride {
    public:
    TmoOverride(const std::shared_ptr<ITmoBase>& spTmoBase);

    bool RequestOverride() override;
    bool ReleaseOverride() override;

    bool SetBypass(bool bypass) override;
    bool SetRegionOfInterest(intel_vvp_tmo_roi roi) override;
    bool SetEnableRoi(bool enable) override;
    bool SetRoiOutside(bool outside) override;
    bool SetThreshold(uint32_t threshold) override;
    bool SetLevel(uint32_t level) override;

    private:
    std::shared_ptr<ITmoBase> _spTmoBase;
    bool _overriding = false;
};

} // namespace Tmo

} // namespace SwApi