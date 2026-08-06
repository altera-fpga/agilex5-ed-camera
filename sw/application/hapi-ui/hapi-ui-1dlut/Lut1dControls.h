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
#include "Lut1D.h"
#include "HistogramStats.h"
#include "ITmo.h"

using namespace SwApi::Lut1dUtils;

#define MAX_BARS 256
#define MAX_SEGMENTS 8


class CustomCurveInfo
{
    public:
    uint16_t num_bars;
    uint32_t values[2][MAX_BARS];
    std::string names[2];

    uint16_t num_segments;
    float control_points[MAX_SEGMENTS][2];
    float knots[MAX_SEGMENTS + 1][2];

    static CustomCurveInfo FromString(bool csv, const std::string& strValue);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    std::string ToString(bool csv);

    private:
    std::string ValuesString();
    std::string PointsString();
};

class Lut1dRoI
{
    public:
    static Lut1dRoI FromString(bool csv, const std::string& strValue);
    void GetJSON(AtUtils::IJsonObjectPtr& spJsonObject);
    std::string ToString(bool csv);

    bool _enabled = true;
    bool _outside = false;
    bool _tmo_enabled = true;
    float _x;
    float _y;
    float _width;
    float _height;
};

/// Encapsulates UI functionality for the 1D LUT IP
class Lut1dControls : public IpUiControls
{
    using CurveEditorCustomControl = UiCustomControlWithValue<CustomCurveInfo>;
    using RoiCustomControl = UiCustomControlWithValue<Lut1dRoI>;
public:
    /// Creates an instance of the class that'll manage the given 1D LUT instance.
    Lut1dControls(const std::shared_ptr<SwApi::Lut1D>& spLut1d,
                const std::shared_ptr<SwApi::HistogramStats>& spHs,
                const std::shared_ptr<SwApi::ITmoOverride>& spTmo,
                bool enableDebugUi = false, bool bypassByDefault = true, const std::string& title = "1D LUT");
    ~Lut1dControls();
    Lut1dControls(const Lut1dControls& other) = delete;
    Lut1dControls& operator=(const Lut1dControls& other) = delete;
    Lut1dControls(Lut1dControls&& other) = delete;
    Lut1dControls& operator=(Lut1dControls&& other) = delete;

    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
    std::vector<std::shared_ptr<UiControlContainer>> CreateCustomControls();

    std::string GetSettingsSectionName() override { return _settingName; };
    void SetSettingSectionName(const std::string& settingName);

    void StatsUpdateLoop();

    void AutoUpdateCallback();
    void SetAutoUpdateToggleFunc(std::function<void(bool)> fpLut1dAutoControlToggleCb);

    void Enable(bool v);

private:
    void WorkOutWhichElementsToEnable();
    CustomCurveInfo GetContainer();
    void ResetCurve();
    void UpdateRoi(bool updateUI, bool readHs);
    void UploadLuts(uint32_t clientID);

    bool _gammaLayerEnabled = false;
    bool _customLayerEnabled = false;

    std::shared_ptr<SwApi::Lut1D> _spLut1d;
    std::shared_ptr<SwApi::HistogramStats> _spHs;
    std::shared_ptr<SwApi::ITmoOverride> _spTmo;

    std::string _title;
    std::string _settingName;

    std::shared_ptr<UiControlItemBoolean>       _spBypass;

    // Layer controls
    std::shared_ptr<UiControlItemBoolean>       _spEnableLayer;
    std::shared_ptr<UiControlItemBoolean>       _spEnableCustomLayer;

    // Generation Controls
    std::shared_ptr<UiControlContainer>         _spGenPanel;
    std::shared_ptr<UiControlItemEnum>          _spCurveType;
    std::shared_ptr<UiControlItemEnum>          _spTransferFunc;
    std::shared_ptr<UiControlItemSlider>        _spGamma;
    std::shared_ptr<UiControlItemSlider>        _spBlackLum;
    std::shared_ptr<UiControlItemSlider>        _spWhiteLum;

    // Start with default gamma layer
    std::vector<string> _layerNames = {"Gamma", "Custom"};

    std::vector<UiEnumOption> _curveOptions = 
    {
        {"Gamma", (uint32_t)TransferFunctionCurve::Gamma},
        {"HLG", (uint32_t)TransferFunctionCurve::HLG},
        {"PQ", (uint32_t)TransferFunctionCurve::PQ}
    };

    std::vector<UiEnumOption> _funcTypeOptions = 
    {
        {"OETF", (uint32_t)TransferFunctionType::OETF},
        {"OETF Legacy", (uint32_t)TransferFunctionType::OETF_LEGACY},
        {"EOTF", (uint32_t)TransferFunctionType::EOTF},
        {"OOTF", (uint32_t)TransferFunctionType::OOTF},
        // {"IEOTF", (uint32_t)TransferFunctionType::IEOTF} // Added during runtime when curve type is not Gamma
    };

    std::shared_ptr<UiControlItemButton>        _spCustomiseCurve;

    std::shared_ptr<UiControlItemLabel>         _spFrameStats;

    // Custom Curve Controls
    std::shared_ptr<UiControlContainer>         _spCustomControlsPanel;
    std::shared_ptr<UiControlContainer>         _spCustomRoiPanel; 
	std::shared_ptr<UiControlContainer>         _spCustomCurvePanel;
    std::shared_ptr<UiControlItemBoolean>       _spAutoUpdate;
    std::shared_ptr<UiControlItemEnum>          _spChartType;
    std::shared_ptr<UiControlItemSlider>        _spNumPoints;
    std::shared_ptr<UiControlItemButton>        _spReset;
    std::shared_ptr<UiControlItemFloat>         _spX;
    std::shared_ptr<UiControlItemFloat>         _spY;
    std::shared_ptr<UiControlItemFloat>         _spWidth;
    std::shared_ptr<UiControlItemFloat>         _spHeight;
    std::shared_ptr<UiControlItemBoolean>       _spHighlightRoi;
    std::shared_ptr<RoiCustomControl>           _spCustomRoi;
    std::shared_ptr<CurveEditorCustomControl>   _spCustomCurve;

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

    bool _ready = false;
    bool _enableDebugUi = false;

    // Generation Variables
    uint32_t _currentLayer = 0;
    TransferFunctionCurve _curve = TransferFunctionCurve::Gamma;
    TransferFunctionType _type = TransferFunctionType::OETF;

    // Curve Editor Variables
    DisplayType _chartType = DisplayType::Frame;
    uint16_t _numSegments = 3;
    std::vector<std::pair<FloatT, FloatT>> _controlPoints;
    std::vector<std::pair<FloatT, FloatT>> _knots;
    Lut1dRoI _roi;
    bool _highlighting = true;

    std::function<void(bool)> _fpLut1dAutoControlToggleCb;

    mutable std::mutex _histogramDataMutex;
    std::shared_ptr<SwApi::Hs::TableResults> _spLatestHistogramData;
    SwApi::HistogramStats::HistogramSubscriptionId _histogramSubscriptionId = 0;

    bool _bypassByDefault = true;

    std::shared_ptr<UiControlContainer> _spContainer;
};

class Show1dLutDialogUiUpdate : public ModalDialogUiUpdate
{
public:
    Show1dLutDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                             uint32_t clientID = allClients, DialogClosedCB closedCB = nullptr, std::string title = "");
};