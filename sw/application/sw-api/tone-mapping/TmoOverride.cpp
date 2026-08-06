/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "TmoOverride.h"

namespace SwApi {

std::shared_ptr<ITmoOverride> ITmoOverride::Create(const std::shared_ptr<ITmoBase>& spTmoBase)
{
    return std::make_shared<Tmo::TmoOverride>(spTmoBase);
}

namespace Tmo {

TmoOverride::TmoOverride(const std::shared_ptr<ITmoBase>& spTmoBase)  : _spTmoBase(spTmoBase)
{}

bool TmoOverride::RequestOverride()
{
    if (!_spTmoBase->GetOverride())//If the TMO is not currently being overridden elsewhere 
    {
        bool ret =  _spTmoBase->SetOverride(true);
        if (ret) _overriding = true;
        return ret;
    }
    return false;
}

bool TmoOverride::ReleaseOverride()
{
    //Sanity check - we should only release override if we have it
    if (_overriding)
    {
        _overriding = false;
        return _spTmoBase->SetOverride(false);
    }
    return false;
}

bool TmoOverride::SetBypass(bool bypass)
{
    if (_overriding)
    {
        _spTmoBase->SetBypass(bypass);
        return true;
    }
    return false;
}

bool TmoOverride::SetRegionOfInterest(intel_vvp_tmo_roi roi)
{
    if (_overriding)
    {
        return _spTmoBase->SetRegionOfInterest(roi);
    }
    return false;
}

bool TmoOverride::SetEnableRoi(bool enable)
{
    if (_overriding)
    {
        return _spTmoBase->SetEnableRoi(enable);
    }
    return false;
}

bool TmoOverride::SetRoiOutside(bool outside)
{
    if (_overriding)
    {
        _spTmoBase->SetRoiOutside(outside);
        return true;
    }
    return false;
}

bool TmoOverride::SetThreshold(uint32_t threshold)
{
    if (_overriding)
    {
        _spTmoBase->SetThreshold(threshold);
        return true;
    }
    return false;
}

bool TmoOverride::SetLevel(uint32_t level)
{
    if (_overriding)
    {
        _spTmoBase->SetLevel(level);
        return true;
    }
    return false;
}

} // namespace Tmo

} // namespace SwApi
