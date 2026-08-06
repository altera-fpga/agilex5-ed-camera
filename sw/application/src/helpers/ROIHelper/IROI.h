/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include "HapiVvpROI.h"

class IROI
{
public:
    IROI() {};
    virtual ~IROI() {};

    virtual void SetResolution(uint32_t newWidth, uint32_t newHeight) = 0;
    virtual bool SetRegionOfInterest(altera_vvp_roi roi) = 0;
    virtual bool SetEnableRoi(bool enable) = 0;
};

using IROIPtr = std::shared_ptr<IROI>;