/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "Bls.h"
#include "IpUiControls.h"
#include "IspCommon.h"

class BLSChartInfo
{
    public:
    bool update_colours;
    int32_t max_value;
    int32_t min_value;
    uint32_t values[4];
    std::string chart_names[4];
    std::string chart_colours[4];

    static BLSChartInfo FromString(bool csv, const std::string& strValue);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    std::string ToString(bool csv);

    private:
    std::string ValuesString();
};

/// Encapsulates UI functionality for the Black Level Statistics IP
class BlsControls : public IpUiControls
{
    using GraphCustomControl = UiCustomControlWithValue<BLSChartInfo>;
    public:
        /// Creates an instance of the class that'll manage the given BLC instance.
        BlsControls(const std::shared_ptr<SwApi::Bls>& spBls, bool enableDebugUi = true);

        /// Generates the UI, and returns the resulting container(s)
        std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

        std::string GetSettingsSectionName() override
        {
            return "BlackLevelStatistics";
        };

        BLSChartInfo GetContainer();

        void UpdateGraphValues(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t target);

    private:

        std::shared_ptr<SwApi::Bls> _spBls;

        bool _enableDebugUi = false;
        std::shared_ptr<GraphCustomControl> _spGraphCustomControl;

        void PopulateMinMaxValues(BLSChartInfo& info, uint32_t target);


};
