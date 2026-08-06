/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "ROIHelper.h"
#include <cassert>

std::shared_ptr<IROI> ROIHelper::Create(Hapi::VvpROIPtr spVvpRoi)
{
    return std::make_shared<ROIHelper>(spVvpRoi);
}

ROIHelper::ROIHelper(Hapi::VvpROIPtr spVvpROI)
: _spVvpROI(spVvpROI)
{
    altera_vvp_roi_set_mode(_spVvpROI->GetInstance(), 0x0);
}

ROIHelper::~ROIHelper()
{

}

void ROIHelper::SetResolution(uint32_t newWidth, uint32_t newHeight)
{
    altera_vvp_roi_set_resolution(_spVvpROI->GetInstance(), newWidth, newHeight);
}

bool ROIHelper::SetRegionOfInterest(altera_vvp_roi roi)
{
    bool rc = false;
    if(altera_vvp_roi_set_roi(_spVvpROI->GetInstance(), roi) == kIntelVvpCoreOk)
    {
        rc = true;
    }
    return rc;
}

bool ROIHelper::SetEnableRoi(bool enable)
{
    bool rc = false;
    if(altera_vvp_roi_set_enable(_spVvpROI->GetInstance(), enable) == kIntelVvpCoreOk)
    {
        rc = true;
    }
    return rc;
}

