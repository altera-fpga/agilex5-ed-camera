/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "TmoUtils.h"

#include <iostream>

namespace SwApi {

    TmoStatus::TmoStatus()
    : _roi{0, 0, 0, 0}
    , _bypass(false)
    , _enable_roi(false)
    , _outside(false)
    , _threshold(0)
    , _level(0)
    {}

    TmoStatus::TmoStatus(const TmoStatus& new_status)
    {
        _roi = new_status._roi;
        _bypass = new_status._bypass;
        _enable_roi = new_status._enable_roi;
        _outside = new_status._outside;
        _threshold = new_status._threshold;
        _level = new_status._level;
    }
    
    TmoStatus& TmoStatus::operator=(const TmoStatus& new_status)
    {
        if (this != &new_status)
        {
            _roi = new_status._roi;
            _bypass = new_status._bypass;
            _enable_roi = new_status._enable_roi;
            _outside = new_status._outside;
            _threshold = new_status._threshold;
            _level = new_status._level;
        }
        return *this;
    }

    TmoStatus::TmoStatus(TmoStatus&& new_status)
    {
        _roi = new_status._roi;
        _bypass = new_status._bypass;
        _enable_roi = new_status._enable_roi;
        _outside = new_status._outside;
        _threshold = new_status._threshold;
        _level = new_status._level;
    }

    TmoStatus& TmoStatus::operator=(TmoStatus&& new_status)
    {
        if (this != &new_status)
        {
            _roi = new_status._roi;
            _bypass = new_status._bypass;
            _enable_roi = new_status._enable_roi;
            _outside = new_status._outside;
            _threshold = new_status._threshold;
            _level = new_status._level;
        }
        return *this;
    }
    
} // namespace SwApi