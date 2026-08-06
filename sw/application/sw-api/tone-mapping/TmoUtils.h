/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpTmo.h"
#include <iosfwd>
#include <cstdint>

namespace SwApi {

    struct TmoStatus
    {
        intel_vvp_tmo_roi _roi;
        bool _bypass;
        bool _enable_roi;
        bool _outside;
        uint32_t _threshold;
        uint32_t _level;

        TmoStatus();
        ~TmoStatus(){}
        TmoStatus(const TmoStatus& new_status);
        TmoStatus& operator=(const TmoStatus& new_status);
        TmoStatus(TmoStatus&& new_status);
        TmoStatus& operator=(TmoStatus&& new_status);
    };

} // namespace SwApi
