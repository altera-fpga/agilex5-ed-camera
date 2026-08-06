/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "IpUiControls.h"

#include "TmoOverride.h"
#include "HistogramStats.h"
#include "Bls.h"
#include "Wbs.h"
#include "IspCommon.h"

class VvpIspProfileMasterRegionControls : public IpUiControls
{
    
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    VvpIspProfileMasterRegionControls(const std::shared_ptr<SwApi::ITmoOverride>& spTmoOverride,
                                      const std::shared_ptr<SwApi::HistogramStats>& spHs,
                                      const std::shared_ptr<SwApi::Bls>& spBls,
                                      const std::shared_ptr<SwApi::Wbs>& spWbs);

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "VvpIspProfileMasterRegion"; };

    void UpdateAllCoreRoI();

private:

    std::shared_ptr<SwApi::ITmoOverride> _spTmoOverride;
    std::shared_ptr<SwApi::HistogramStats> _spHs;
    std::shared_ptr<SwApi::Bls> _spBls;
    std::shared_ptr<SwApi::Wbs> _spWbs;

    RoiSelectorData _roi;

    using RoiCustomControl = UiCustomControlWithValue<RoiSelectorData>;
    std::shared_ptr<RoiCustomControl> _spRoiCustomControl;

};
