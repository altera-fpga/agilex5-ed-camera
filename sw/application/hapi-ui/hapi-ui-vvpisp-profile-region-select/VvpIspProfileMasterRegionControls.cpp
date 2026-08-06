/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspProfileMasterRegionControls.h"
#include "UiElements.h"
#include "BlsUtils.h"

#include <string>

static const uint32_t panelWidth = 450;
static const uint32_t margin = 12;

VvpIspProfileMasterRegionControls::VvpIspProfileMasterRegionControls(const std::shared_ptr<SwApi::ITmoOverride>& spTmoOverride,
                                                                     const std::shared_ptr<SwApi::HistogramStats>& spHs,
                                                                     const std::shared_ptr<SwApi::Bls>& spBls,
                                                                     const std::shared_ptr<SwApi::Wbs>& spWbs)
: _spTmoOverride(spTmoOverride),
  _spHs(spHs),
  _spBls(spBls),
  _spWbs(spWbs)
{
    _roi._x = 0;
    _roi._y = 0;
    _roi._width = 1.0;
    _roi._height = 1.0;
    _roi._enabled = false;
    _roi._tmo_enabled = false;
    _roi._outside = false;
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileMasterRegionControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Profile Global Region Select", GetSettingsSectionName());
    
    auto roiUpdateCB = [this](uint32_t clientId, RoiSelectorData& value) 
    {
        _roi._x = value._x;
        _roi._y = value._y;
        _roi._width = value._width;
        _roi._height = value._height;

        UpdateAllCoreRoI();
    };

    _spRoiCustomControl = std::make_shared<RoiCustomControl>("OJAWBRoiControl", _roi, roiUpdateCB);
    spContainer->Add(_spRoiCustomControl);

    _roi._enabled = true;
    _roi._tmo_enabled = true;
    _roi._outside = true;

    _spRoiCustomControl->UpdateValue(_roi);

    _spTmoOverride->ReleaseOverride();

    return {std::move(spContainer)};
}

void VvpIspProfileMasterRegionControls::UpdateAllCoreRoI()
{
    // BLS
    if(_spBls)
    {
        SwApi::BlackRegion blsRoI;

        const auto [blsInWidth, blsInHeight] = _spBls->GetResolution();

        blsRoI.h_start = (_roi._x) * blsInWidth;
        blsRoI.h_end = (_roi._x + _roi._width) * blsInWidth;
        blsRoI.v_start = (_roi._y) * blsInHeight;
        blsRoI.v_end = (_roi._y + _roi._height) * blsInHeight;

        _spBls->SetCombinedOpticalBlackRegion(blsRoI);
    }
    // WBS
    if(_spWbs)
    {
        SwApi::WbsRoI wbsRoI;

        const auto [wbsInWidth, wbsInHeight] = _spWbs->GetResolution();

        wbsRoI.h_start = (_roi._x) * wbsInWidth;
        wbsRoI.h_end = (_roi._x + _roi._width) * wbsInWidth;
        wbsRoI.v_start = (_roi._y) * wbsInHeight;
        wbsRoI.v_end = (_roi._y + _roi._height) * wbsInHeight;

        _spWbs->SetRoI(wbsRoI);
    }
    // HS
    if(_spHs)
    {
        SwApi::Hs::RegionOfInterest hsRoI;

        const auto [hsInWidth, hsInHeight] = _spHs->GetResolution();

        hsRoI.h_start = (_roi._x) * hsInWidth;
        hsRoI.h_end = (_roi._x + _roi._width) * hsInWidth;
        hsRoI.v_start = (_roi._y) * hsInHeight;
        hsRoI.v_end = (_roi._y + _roi._height) * hsInHeight;

        _spHs->SetRegionOfInterest(hsRoI);
    }

    // TMO
    intel_vvp_tmo_roi wbRoi = {
        ._x = _roi._x,
        ._y = _roi._y,
        ._width = _roi._width,
        ._height = _roi._height
    };
    _spTmoOverride->SetRegionOfInterest(wbRoi);
}
