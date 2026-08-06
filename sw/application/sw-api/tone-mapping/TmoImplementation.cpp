/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "TmoImplementation.h"

namespace SwApi {

std::shared_ptr<ITmo> ITmo::Create(const std::shared_ptr<ITmoBase>& spTmoBase)
{
    return std::make_shared<Tmo::TmoImplementation>(spTmoBase);
}

namespace Tmo {

TmoImplementation::TmoImplementation(const std::shared_ptr<ITmoBase>& spTmoBase)
                                     : _spTmoBase(spTmoBase)
{}

bool TmoImplementation::SetBypass(bool bypass) {
    if (!_spTmoBase->GetOverride())
    {
        _spTmoBase->SetBypass(bypass);
        return true;
    }
    return false;
}

bool TmoImplementation::GetBypass() {
    return _spTmoBase->GetBypass();
}

bool TmoImplementation::SetOutputWidth(uint32_t newWidth) {
    if (!_spTmoBase->GetOverride())
    {
        return _spTmoBase->SetOutputWidth(newWidth);
    }
    return false;
}

uint32_t TmoImplementation::GetOutputWidth() {
    return _spTmoBase->GetOutputWidth();
}

bool TmoImplementation::SetOutputHeight(uint32_t newHeight) {
    if (!_spTmoBase->GetOverride())
    {
        return _spTmoBase->SetOutputHeight(newHeight);
    }
    return false;
}

uint32_t TmoImplementation::GetOutputHeight() {
    return _spTmoBase->GetOutputHeight();
}

bool TmoImplementation::SetRegionOfInterest(intel_vvp_tmo_roi roi)
{
    if (!_spTmoBase->GetOverride())
    {
        return _spTmoBase->SetRegionOfInterest(roi);
    }
    return false;
}

bool TmoImplementation::SetEnableRoi(bool enable)
{
    if (!_spTmoBase->GetOverride())
    {
        return _spTmoBase->SetEnableRoi(enable);
    }
    return false;
}

bool TmoImplementation::SetRoiOutside(bool outside)
{
    if (!_spTmoBase->GetOverride())
    {
        _spTmoBase->SetRoiOutside(outside);
        return true;
    }
    return false;
}

bool TmoImplementation::SetThreshold(uint32_t threshold)
{
    if (!_spTmoBase->GetOverride())
    {
        _spTmoBase->SetThreshold(threshold);
        return true;
    }
    return false;
}

bool TmoImplementation::SetLevel(uint32_t level)
{
    if (!_spTmoBase->GetOverride())
    {
        _spTmoBase->SetLevel(level);
        return true;
    }
    return false;
}

} // namespace Tmo

} // namespace SwApi
