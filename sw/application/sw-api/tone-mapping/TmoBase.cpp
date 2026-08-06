/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "TmoBase.h"

namespace SwApi {

std::shared_ptr<ITmoBase> ITmoBase::Create(const Hapi::VvpTmoPtr& spTmo,
                                   uint32_t initialOutputWidth,
                                   uint32_t initialOutputHeight) {
    return std::make_shared<Tmo::TmoBase>(spTmo, initialOutputWidth, initialOutputHeight);
}

namespace Tmo {

TmoBase::TmoBase(const Hapi::VvpTmoPtr& spTmo,
                                    uint32_t initialOutputWidth,
                                    uint32_t initialOutputHeight)
                                    : _spTmo(spTmo)
{
    // Set the output width and height.
    this->SetOutputWidth(initialOutputWidth);
    this->SetOutputHeight(initialOutputHeight);
    
    //Record initial status
    intel_vvp_tmo_get_region_of_interest(_spTmo->GetInstance(), &_current_status._roi, 
                                        &_current_status._enable_roi, &_current_status._outside);
    _current_status._bypass = GetBypass();
    _current_status._threshold = intel_vvp_tmo_get_threshold(_spTmo->GetInstance());
    _current_status._level = intel_vvp_tmo_get_volume(_spTmo->GetInstance());
}

void TmoBase::SetBypass(bool bypass) {
    intel_vvp_tmo_set_bypass(_spTmo->GetInstance(), bypass);
    _current_status._bypass = bypass;
}

bool TmoBase::GetBypass() {
    return _current_status._bypass;
}

bool TmoBase::SetOutputWidth(uint32_t newWidth) {
    auto ret = intel_vvp_core_set_img_info_width(_spTmo->GetInstance(), newWidth);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        _outputWidth = newWidth;
        return true;
    } else {
        return false;
    }
}

uint32_t TmoBase::GetOutputWidth() {
    return _outputWidth;
}

bool TmoBase::SetOutputHeight(uint32_t newHeight) {
    auto ret = intel_vvp_core_set_img_info_height(_spTmo->GetInstance(), newHeight);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        _outputHeight = newHeight;
        return true;
    } else {
        return false;
    }
}

uint32_t TmoBase::GetOutputHeight() {
    return _outputHeight;
}

void TmoBase::SetResolution(uint32_t newWidth, uint32_t newHeight) {
    intel_vvp_tmo_set_resolution(_spTmo->GetInstance(), newWidth, newHeight);
}

bool TmoBase::SetRegionOfInterest(intel_vvp_tmo_roi roi)
{
    bool ret = intel_vvp_tmo_set_region_of_interest(_spTmo->GetInstance(), roi);
    if (ret)
    {
        _current_status._roi = roi;
        return true;
    }
    return false;
}

bool TmoBase::SetEnableRoi(bool enable)
{
    bool ret = intel_vvp_tmo_enable_region_of_interest(_spTmo->GetInstance(), enable);
    if (ret)
    {
        _current_status._enable_roi = enable;
        return true;
    }
    return false;
}

void TmoBase::SetRoiOutside(bool outside)
{
    intel_vvp_tmo_set_region_of_interest_outside(_spTmo->GetInstance(), outside);
    _current_status._outside = outside;
}

void TmoBase::SetThreshold(uint32_t threshold)
{
    intel_vvp_tmo_set_threshold(_spTmo->GetInstance(), threshold);
    _current_status._threshold = threshold;
}

void TmoBase::SetLevel(uint32_t level)
{
    intel_vvp_tmo_set_volume(_spTmo->GetInstance(), level);
    _current_status._level = level;
}

bool TmoBase::SetOverride(bool override)
{
    if(this->_override == override) return false; // To prevent _stored_status being overwritten

    this->_override = override;

    if (override)
    {
        _stored_status = _current_status;
    }
    else
    {
        _current_status = _stored_status;
        UpdateCore();
    }

    return true;
}

bool TmoBase::GetOverride()
{
    return _override;
}

void TmoBase::UpdateCore()
{
    intel_vvp_tmo_set_bypass(_spTmo->GetInstance(), _current_status._bypass);
    intel_vvp_tmo_set_threshold(_spTmo->GetInstance(), _current_status._threshold);
    intel_vvp_tmo_set_volume(_spTmo->GetInstance(), _current_status._level);
    intel_vvp_tmo_set_region_of_interest(_spTmo->GetInstance(), _current_status._roi);
    intel_vvp_tmo_enable_region_of_interest(_spTmo->GetInstance(), _current_status._enable_roi);
    intel_vvp_tmo_set_region_of_interest_outside(_spTmo->GetInstance(), _current_status._outside);
}

}

}