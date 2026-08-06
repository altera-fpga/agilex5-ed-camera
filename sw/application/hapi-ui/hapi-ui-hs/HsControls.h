/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include <mutex>

#include "IpUiControls.h"
#include "HistogramStats.h"
#include "IROI.h"
#include "IspCommon.h"

#define MAX_BARS 256

class ChartInfo
{
    public:
    uint16_t num_bars;
    uint32_t values[2][MAX_BARS];
    std::string names[2];

    static ChartInfo FromString(bool csv, const std::string& strValue);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    std::string ToString(bool csv);

    private:
    std::string ValuesString();
};

/// Encapsulates UI functionality for the Histogram Statistics IP
class HsControls : public IpUiControls
{
    using GraphCustomControl = UiCustomControlWithValue<ChartInfo>;
    using RoiCustomControl = UiCustomControlWithValue<RoiSelectorData>;
public:
    /// Creates an instance of the class that'll manage the given BLC instance.
    HsControls(const std::string& name, 
                const std::shared_ptr<SwApi::HistogramStats>& spHs,
                const IROIPtr& spIROI,
                bool enableDebugUi = true);
    ~HsControls();
    HsControls(const HsControls& other) = delete;
    HsControls& operator=(const HsControls& other) = delete;
    HsControls(HsControls&& other) = delete;
    HsControls& operator=(HsControls&& other) = delete;
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void AutoUpdateCallback();
    void SetAutoUpdateToggleFunc(std::function<void(bool)> fpHsAutoControlToggleCb);

private:

    ChartInfo GetContainer(const std::shared_ptr<SwApi::Hs::TableResults>& histogramData);
    void UpdateStats(ChartInfo container);
    void UpdateRoi(bool updateUI, bool readHs);

    const std::string _name; 

    std::shared_ptr<SwApi::HistogramStats>  _spHs;
    IROIPtr                                 _spIROI;

	std::shared_ptr<UiControlContainer>     _spChartPanel;
    std::shared_ptr<UiControlContainer>     _spStatsUpdatePanel;
    std::shared_ptr<UiControlContainer>     _spStatsConfigPanel;

    std::shared_ptr<UiControlContainer>     _spRoiPanel;
    std::shared_ptr<UiControlContainer>     _spRoiEditorPanel;
    std::shared_ptr<UiControlContainer>     _spRoiXYPanel;
    std::shared_ptr<UiControlContainer>     _spRoiWHPanel;
    std::shared_ptr<UiControlContainer>     _spRoiHighlightPanel;

    // Stats controls
    std::shared_ptr<UiControlItemLabel>     _spMode;
    std::shared_ptr<UiControlItemLabel>     _spStandardDeviation;
    std::shared_ptr<UiControlItemEnum>      _spDisplayType;

    enum DisplayType {
        Frame,
        Roi,
        Overlay
    };
    std::vector<UiEnumOption> _displayOptions = {
        {"Full Frame", (uint32_t)DisplayType::Frame},
        {"Region of Interest", (uint32_t)DisplayType::Roi},
        {"Both Overlayed", (uint32_t)DisplayType::Overlay}
    };

    // Roi controls
    std::shared_ptr<UiControlItemBoolean>   _spHighlightRoi; 
    std::shared_ptr<UiControlItemFloat>     _spX;
    std::shared_ptr<UiControlItemFloat>     _spY;
    std::shared_ptr<UiControlItemFloat>     _spWidth;
    std::shared_ptr<UiControlItemFloat>     _spHeight;
    std::shared_ptr<RoiCustomControl>       _spRoiCustomControl;

    // Histogram
    std::shared_ptr<GraphCustomControl>     _spGraphCustomControl;

    RoiSelectorData _roi;
    DisplayType _displayType;
    bool _ready = false;
    bool _highlighting = false;
    bool _pendingHwUpdate = false;

    bool _enableDebugUi = false;

    mutable std::mutex _histogramDataMutex;
    std::shared_ptr<SwApi::Hs::TableResults> _spLatestHistogramData;
    SwApi::HistogramStats::HistogramSubscriptionId _histogramSubscriptionId = 0;

    std::function<void(bool)> _fpHsAutoControlToggleCb;
};

class ShowRoiDialogUiUpdate : public ModalDialogUiUpdate
{
public:
    ShowRoiDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                             uint32_t clientID = allClients, DialogClosedCB closedCB = nullptr);
};
