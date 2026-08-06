/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include "IROI.h"
#include "HapiVvpROI.h"

class ROIHelper : public IROI
{
public:
    static IROIPtr Create(Hapi::VvpROIPtr spVvpRoi);

    ROIHelper(Hapi::VvpROIPtr spVvpROI);
    virtual ~ROIHelper();

    virtual void SetResolution(uint32_t newWidth, uint32_t newHeight) override;

    virtual bool SetRegionOfInterest(altera_vvp_roi roi) override;
    virtual bool SetEnableRoi(bool enable) override;

private:
    Hapi::VvpROIPtr _spVvpROI;
};