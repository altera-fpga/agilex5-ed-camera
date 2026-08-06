/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "HsPreview.h"


HistogramPreview::HistogramPreview(const std::string& name)
: _name(name)
{

}


HistogramPreview::~HistogramPreview()
{
    
}


std::vector<std::shared_ptr<UiControlContainer>> HistogramPreview::AddUiElements()
{
    auto container = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    _spGraphCustomControl = std::make_shared<GraphCustomControl>("OJBarChartControl", ChartInfo{}, [](uint32_t clientId, const ChartInfo& value){});
    container->Add(_spGraphCustomControl);

    return {std::move(container)};
}


ChartInfo HistogramPreview::GetUiHsData(const SwApi::Hs::TableResults& hsData)
{
    // Note: data set names left blank intentionally
    // so that they are not displayed in the small histogram preview tile
    ChartInfo container{};

    if(hsData.valid)
    {
        // Only display full frame histogram in the histogram preview
        container.num_bars = hsData.frame_luma_bins.num_luma_bins;
        for (int i = 0; i < container.num_bars; i++)
        {
            container.values[0][i] = hsData.frame_luma_bins.luma_bins[i];
            container.values[1][i] = 0;
        }
    }

    return container;
}


void HistogramPreview::Update(const SwApi::Hs::TableResults& hsData)
{
    ChartInfo uiHsData = GetUiHsData(hsData);
    _spGraphCustomControl->UpdateValue(uiHsData);    
}