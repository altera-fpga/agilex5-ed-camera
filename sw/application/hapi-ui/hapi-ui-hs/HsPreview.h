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
#include "HsControls.h"
#include "HistogramStatsTypes.h"


class HistogramPreview : public IpUiControls
{
public:
    using GraphCustomControl = UiCustomControlWithValue<ChartInfo>;

    HistogramPreview(const std::string& name);
    virtual ~HistogramPreview();

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name + "Preview"; }

    void Update(const SwApi::Hs::TableResults& hsData);

private:

    ChartInfo GetUiHsData(const SwApi::Hs::TableResults& hsData);

    const std::string _name;
    // Histogram
    std::shared_ptr<GraphCustomControl>     _spGraphCustomControl;
};