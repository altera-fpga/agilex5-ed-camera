/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "Wbs.h"

#include "IpUiControls.h"

#define WBS_NUM_ZONES 49

struct WBSRatioChartInfo
{
    public:
    uint8_t values_r[WBS_NUM_ZONES];
    uint8_t values_g[WBS_NUM_ZONES];
    uint8_t values_b[WBS_NUM_ZONES];

    uint32_t values_count[WBS_NUM_ZONES];

    static WBSRatioChartInfo FromString(bool csv, const std::string& strValue);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    std::string ToString(bool csv);

    private:
    std::string ValuesString();
};

class WbsControls : public IpUiControls
{
    using CustomRatioGraph = UiCustomControlWithValue<WBSRatioChartInfo>;

public:
    /// Creates an instance of the class that'll manage the given WBC instance.
    WbsControls(const std::string& name, const std::shared_ptr<SwApi::Wbs>& spWbs, bool enableDebugUi = false, bool powerUser = false, bool flipDisplay = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void LoadSwitchRouterFunc(std::function<void(bool state)> func)
    {
        _setSwitchRouterCB = std::move(func);
    };

private:

    WBSRatioChartInfo GetContainer(bool flip);

    const std::string _name; 

    std::shared_ptr<SwApi::Wbs> _spWbs;
    bool _enableDebugUi = false;
    bool _powerUser = false;
    bool _flipDisplay = false;

    std::shared_ptr<CustomRatioGraph> _spGraphCustomControl;

    std::function<void(bool state)> _setSwitchRouterCB;

};

