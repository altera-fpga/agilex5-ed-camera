/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "Lut1dControls.h"
#include "UiElements.h"
#include <cmath>


static constexpr uint32_t GAMMA_LAYER_IDX = 0;
static constexpr uint32_t CUSTOM_LAYER_IDX = 1;


Lut1dControls::Lut1dControls(const std::shared_ptr<SwApi::Lut1D>& spLut1d, 
                            const std::shared_ptr<SwApi::HistogramStats>& spHs,
                            const std::shared_ptr<SwApi::ITmoOverride>& spTmo,
                            bool enableDebugUi, bool bypassByDefault, const std::string& title)
    : _spLut1d(spLut1d)
    , _spHs(spHs)
    , _spTmo(spTmo)
    , _title(title)
    , _settingName("LUT1D")
    , _enableDebugUi(enableDebugUi)
    , _bypassByDefault(bypassByDefault)
{
    if (_spHs)
    {
        _histogramSubscriptionId = _spHs->RegisterHistogramObserver([this](const std::shared_ptr<SwApi::Hs::TableResults>& histogramData)
        {
            std::scoped_lock lock(_histogramDataMutex);
            _spLatestHistogramData = histogramData;
        });
    }
}

Lut1dControls::~Lut1dControls()
{
    if (_spHs && _histogramSubscriptionId != 0)
    {
        _spHs->UnregisterHistogramObserver(_histogramSubscriptionId);
    }
}

void Lut1dControls::SetSettingSectionName(const std::string& settingName)
{
    _settingName = settingName;
}

void Lut1dControls::AutoUpdateCallback()
{
    if(_spHs)
        UpdateRoi(false, true);
    
    _spCustomCurve->UpdateValue(GetContainer());
}

void Lut1dControls::SetAutoUpdateToggleFunc(std::function<void(bool)> fpLut1dAutoControlToggleCb)
{
    _fpLut1dAutoControlToggleCb = std::move(fpLut1dAutoControlToggleCb);
}

CustomCurveInfo Lut1dControls::GetContainer()
{
    std::shared_ptr<SwApi::Hs::TableResults> hsStats;

    {
        std::scoped_lock lock(_histogramDataMutex);
        hsStats = _spLatestHistogramData;
    }

    if (!hsStats)
    {
        hsStats = std::make_shared<SwApi::Hs::TableResults>(false);
    }

    CustomCurveInfo container =
    {
        .num_bars = hsStats->frame_luma_bins.num_luma_bins,
        .values = {0},
        .names = { "Full Frame", "Region of Interest" },
        .num_segments = _numSegments,
        .control_points = {0},
        .knots = {0}
    };

    if (!_ready)
    {
        return container;
    }

    for (int i = 0; i < container.num_bars; i++)
    {
        switch (_chartType)
        {
            case DisplayType::Frame:
            {
                container.values[0][i] = hsStats->frame_luma_bins.luma_bins[i];
                // container.values[1][i] = 0;
                break;
            }
            case DisplayType::Roi:
            {
                // container.values[0][i] = 0;
                container.values[1][i] = hsStats->roi_luma_bins.luma_bins[i];
                break;
            }
            case DisplayType::Overlay:
            {
                container.values[0][i] = hsStats->frame_luma_bins.luma_bins[i];
                container.values[1][i] = hsStats->roi_luma_bins.luma_bins[i];
                break;
            }
            default:
            {
                std::cout << "[1DLUT] Error: Invalid Display Type" << std::endl;
            }
        }
    }

    for (int i = 0; i < _numSegments; i++)
    {
        container.control_points[i][0] = _controlPoints[i * 2 + 1].first; // Convert from 2 points per seg to one point
        container.control_points[i][1] = _controlPoints[i * 2 + 1].second;
    }

    for (int i = 0; i < _numSegments + 1; i++)
    {
        container.knots[i][0] = _knots[i].first;
        container.knots[i][1] = _knots[i].second;
    }

    return container;
}

void Lut1dControls::ResetCurve()
{
    _knots.clear();
    for (int i = 0; i < _numSegments + 1; i++)
    {
        _knots.push_back({(FloatT)i / _numSegments, (FloatT)i / _numSegments});
    }

    _controlPoints.clear();
    for (int i = 0; i < _numSegments; i++)
    {
        _controlPoints.push_back({_knots[i].first + (_knots[i + 1].first - _knots[i].first) / 3 ,
                                    _knots[i].second + (_knots[i + 1].second - _knots[i].second) / 3});
        _controlPoints.push_back({_knots[i].first + (_knots[i + 1].first - _knots[i].first) * 2 / 3 ,
                                    _knots[i].second + (_knots[i + 1].second - _knots[i].second) * 2 / 3});
    }
}

void Lut1dControls::UpdateRoi(bool updateUI, bool readHs)
{
    static constexpr float MIN_ROI_SIZE = 0.0625f;

    if (_ready)
    {
        // Make sure all the flags are correct
        _roi._enabled = true;
        _roi._outside = false;
        _roi._tmo_enabled = true;

        const auto [w, h] = _spHs->GetResolution();

        if (readHs)
        {
            // Read the _roi from the driver
            SwApi::Hs::RegionOfInterest pix_roi = _spHs->GetRegionOfInterest();
            _roi._x = (float)pix_roi.h_start / w;
            _roi._y = (float)pix_roi.v_start / h;
            _roi._width = (float)(pix_roi.h_end - pix_roi.h_start) / w;
            _roi._height = (float)(pix_roi.v_end - pix_roi.v_start) / h;

            if (_roi._width < MIN_ROI_SIZE || _roi._height < MIN_ROI_SIZE)
            {
                _roi._x = 0;
                _roi._y = 0;
                _roi._width = 1;
                _roi._height = 1;
                UpdateRoi(true, false);
                return;
            }
        }
        else
        {
            // Push the validated _roi to the driver
            SwApi::Hs::RegionOfInterest pix_roi =
            {
                .h_start = (uint16_t)(_roi._x * w),
                .v_start = (uint16_t)(_roi._y * h),
                .h_end = (uint16_t)((_roi._x + _roi._width) * w),
                .v_end = (uint16_t)((_roi._y + _roi._height) * h)
            };

            _spHs->SetRegionOfInterest(pix_roi);
        }

        if (updateUI)
        {
            _spX->UpdateValue(_roi._x);
            _spY->UpdateValue(_roi._y);
            _spWidth->UpdateValue(_roi._width);
            _spHeight->UpdateValue(_roi._height);

            _spCustomRoi->UpdateValue(_roi);
        }

        if (_highlighting)
        {
            if(_spTmo)
            {
                _spTmo->SetBypass(false);
                _spTmo->SetEnableRoi(true);
                _spTmo->SetRoiOutside(false);
                _spTmo->SetThreshold(9000);
                _spTmo->SetLevel(100);
                intel_vvp_tmo_roi tmo_roi = {
                    ._x = _roi._x,
                    ._y = _roi._y,
                    ._width = _roi._width,
                    ._height = _roi._height
                };
                _spTmo->SetRegionOfInterest(tmo_roi);
            }
        }
    }
}

std::vector<std::shared_ptr<UiControlContainer>> Lut1dControls::CreateCustomControls()
{
    static constexpr uint32_t PANEL_WIDTH = 450;
    static constexpr uint32_t MARGIN = 12;

    std::vector<std::shared_ptr<UiControlContainer>> controls{};

    _spCustomControlsPanel = std::make_shared<UiControlContainer>("Controls", GetSettingsSectionName());

    _spCustomControlsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN });
    _spCustomControlsPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 8 });
    _spCustomControlsPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, PANEL_WIDTH });

    controls.emplace_back(_spCustomControlsPanel);

    if (_spHs)
    {    
        _spCustomRoiPanel = std::make_shared<UiControlContainer>("Region of Interest", GetSettingsSectionName());

        _spCustomRoiPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN });
        _spCustomRoiPanel->SetTopAnchor({ ANCHOR_TYPE::SIBLING_FAR, _spCustomControlsPanel->GetID(), 8 });
        _spCustomRoiPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, PANEL_WIDTH });
        _spCustomRoiPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 480 });

        controls.emplace_back(_spCustomRoiPanel);
    }

    _spCustomCurvePanel = std::make_shared<UiControlContainer>("LUT Curve", GetSettingsSectionName());

    _spCustomCurvePanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN + PANEL_WIDTH + MARGIN });
    _spCustomCurvePanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 8 });
    _spCustomCurvePanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1200 });
    _spCustomCurvePanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 686 });

    controls.emplace_back(_spCustomCurvePanel);

    // Controls

    if (_spHs)
    {    
        auto autoUpdateCB = [this] (uint32_t clientID, bool& value) -> void
        {
            if (_fpLut1dAutoControlToggleCb)
                _fpLut1dAutoControlToggleCb(value);
        };

        auto chartTypeCB = [this] (uint32_t clientID, const UiEnumOption& selectedValue, uint32_t) -> void
        {
            _chartType = (DisplayType)selectedValue._userItemData;

            // Update Bar chart
            _spCustomCurve->UpdateValue(GetContainer());
        };

        _spAutoUpdate = _spCustomControlsPanel->AddBoolControl("Auto Update Stats", 0, autoUpdateCB);
        _spChartType = _spCustomControlsPanel->AddEnumControl("Display Type", 0, _displayOptions, chartTypeCB, 0);
    }

    auto numSegmentsCB = [this] (uint32_t clientID, float& value) -> void
    {
        bool reset = _numSegments != (uint16_t)value;
        _numSegments = (uint16_t)value;

        if (reset)
            ResetCurve();

        _spCustomCurve->UpdateValue(GetContainer());

        _spLut1d->SetKnots(CUSTOM_LAYER_IDX, _knots);
        _spLut1d->SetControlPoints(CUSTOM_LAYER_IDX, _controlPoints);
        UploadLuts(clientID);
    };

    auto resetCB = [this] (uint32_t clientID) -> void
    {
        ResetCurve();
        _spCustomCurve->UpdateValue(GetContainer());

        _spLut1d->SetKnots(CUSTOM_LAYER_IDX, _knots);
        _spLut1d->SetControlPoints(CUSTOM_LAYER_IDX, _controlPoints);
        UploadLuts(clientID);
    };

    _spNumPoints = _spCustomControlsPanel->AddSliderControl("Num Segments", 3.0, 1.0, 8.0, numSegmentsCB, 0);
    _spReset = _spCustomControlsPanel->AddButtonControl("Reset Curve", resetCB);

    if (_spHs)
    {
        // Roi
        auto xCB = [this](uint32_t clientId, float& value)
        {
            _roi._x = value;

            UpdateRoi(true, false);
        };
        _spX = _spCustomRoiPanel->AddFloatControl("X", 0.0f, 0.875, xCB, "roiX", 0.0625f);
        auto yCB = [this](uint32_t clientId, float& value)
        {
            _roi._y = value;

            UpdateRoi(true, false);
        };
        _spY = _spCustomRoiPanel->AddFloatControl("Y", 0.0f, 0.7778f, yCB, "roiY", 0.1111f );
        auto widthCB = [this](uint32_t clientId, float& value)
        {
            _roi._width = value;

            UpdateRoi(true,false );
        };
        _spWidth = _spCustomRoiPanel->AddFloatControl("Width", 0.0625f, 1.0f, widthCB, "roiWidth", 0.75f);
        auto heightCB = [this](uint32_t clientId, float& value)
        {
            _roi._height = value;

            UpdateRoi(true, false);
        };
        _spHeight = _spCustomRoiPanel->AddFloatControl("Height", 0.1111f, 1.0f, heightCB, "roiHeight", 0.5555f);
        auto highlightCB = [this](uint32_t clientID, bool& value)
        {
            if(_spTmo)
            {
                if (value)
                {
                    if (!_highlighting) // If not already overriding
                    {
                        if (!_spTmo->RequestOverride())
                        {
                            return;
                        }
                    }
                }
                else
                {
                    if (_highlighting)
                    {
                        _spTmo->ReleaseOverride();
                    }
                }
                _highlighting = value;
            }

            UpdateRoi(false, true);
        };
        _spHighlightRoi = _spCustomRoiPanel->AddBoolControl("Highlight RoI", highlightCB, "roiHighlight", false);

        auto roiUpdateCB = [this](uint32_t clientId, Lut1dRoI& value)
        {
            _roi._x = value._x;
            _roi._y = value._y;
            _roi._width = value._width;
            _roi._height = value._height;

            _spX->UpdateValue(_roi._x);
            _spY->UpdateValue(_roi._y);
            _spWidth->UpdateValue(_roi._width);
            _spHeight->UpdateValue(_roi._height);

            UpdateRoi(true, false);
        };
        _spCustomRoi = std::make_shared<RoiCustomControl>("OJRoiControl", _roi, roiUpdateCB);
        _spCustomRoiPanel->Add(_spCustomRoi);
    }

    // Curve
    auto curveUpdateCB = [this] (uint32_t clientID, CustomCurveInfo& value) -> void
    {
        _numSegments = value.num_segments;

        _knots.clear();
        for (int i = 0; i < _numSegments + 1; i++)
        {
            _knots.push_back({value.knots[i][0], value.knots[i][1]});
        }

        _controlPoints.clear();
        for (int i = 0; i < _numSegments; i++)
        {
            if (i == 0)
            {
                _controlPoints.push_back({0.0, 0.0});
            }
            else
            {
                // Reflect previous control point over knot
                std::pair<FloatT, FloatT> diff = {_knots[i].first - _controlPoints[i * 2 - 1].first,
                                            _knots[i].second - _controlPoints[i * 2 - 1].second};
                _controlPoints.push_back({_knots[i].first + diff.first, _knots[i].second + diff.second});
            }

            _controlPoints.push_back({value.control_points[i][0], value.control_points[i][1]});
        }

        _spLut1d->SetKnots(CUSTOM_LAYER_IDX, _knots);
        _spLut1d->SetControlPoints(CUSTOM_LAYER_IDX, _controlPoints);
        UploadLuts(clientID);
    };

    _spCustomCurve = std::make_shared<CurveEditorCustomControl>("OJCurveEditorControl", GetContainer(), curveUpdateCB);
    _spCustomCurvePanel->Add(_spCustomCurve);

    ResetCurve();    

    return controls;
}

void Lut1dControls::WorkOutWhichElementsToEnable()
{
    if (!(_spBypass && _spEnableLayer &&
          _spCurveType && _spTransferFunc &&
          _spGamma && _spBlackLum && _spWhiteLum &&
          _spEnableCustomLayer && _spCustomiseCurve))
    {
        return;
    }

    if (!_spLut1d->GetBypass())
    {
        _spEnableLayer->Enable(true);

        if (_gammaLayerEnabled)
        {
            _spCurveType->Enable(true);

            switch (_curve)
            {
                case TransferFunctionCurve::Gamma:
                {
                    _spTransferFunc->Enable(true);
                    _spGamma->Enable(_type != TransferFunctionType::OETF);
                    _spBlackLum->Enable(false);
                    _spWhiteLum->Enable(false);
                    break;
                }
                case TransferFunctionCurve::HLG:
                {
                    _spTransferFunc->Enable(true);

                    bool enable = true;

                    if((_type == TransferFunctionType::OETF) || (_type == TransferFunctionType::OETF_LEGACY))
                        enable = false;

                    _spGamma->Enable(enable);
                    _spBlackLum->Enable(enable);
                    _spWhiteLum->Enable(enable);
                    break;
                }
                case TransferFunctionCurve::PQ:
                {
                    _spTransferFunc->Enable(true);
                    _spGamma->Enable(false);
                    _spBlackLum->Enable(false);
                    _spWhiteLum->Enable(false);
                    break;
                }
                case TransferFunctionCurve::Custom:
                {
                    _spTransferFunc->Enable(false);
                    _spGamma->Enable(false);
                    _spBlackLum->Enable(false);
                    _spWhiteLum->Enable(false);
                    break;
                }
            }
        }
        else
        {
            _spCurveType->Enable(false);
            _spTransferFunc->Enable(false);
            _spGamma->Enable(false);
            _spBlackLum->Enable(false);
            _spWhiteLum->Enable(false);
        }

        _spEnableCustomLayer->Enable(true);
        _spCustomiseCurve->Enable(_customLayerEnabled);
    }
    else
    {
        _spEnableLayer->Enable(false);
        _spCurveType->Enable(false);
        _spTransferFunc->Enable(false);
        _spGamma->Enable(false);
        _spBlackLum->Enable(false);
        _spWhiteLum->Enable(false);
        _spEnableCustomLayer->Enable(false);
        _spCustomiseCurve->Enable(false);
    }
}

void Lut1dControls::UploadLuts(uint32_t clientID)
{
    if (_curve == TransferFunctionCurve::Gamma && _type == TransferFunctionType::IEOTF)
    {
        ModalDialogUiUpdate error_message(clientID, ModalDialogUiUpdate::DialogType::MESSAGE, "Gamma curve cannot be IEOTF", nullptr, nullptr);
        error_message.Notify();
        return;
    }

    if (!_spLut1d->UploadLUTs())
    {
        std::cout << "Upload LUT Failed!\n";
    }
}

std::vector<std::shared_ptr<UiControlContainer>> Lut1dControls::AddUiElements()
{
    static constexpr float GAMMA_DEFAULT_VALUE = 2.2f;

    auto spContainer = std::make_shared<UiControlContainer>(_title, GetSettingsSectionName());
    if (not _spLut1d)
    {
        spContainer->AddLabelControl("1D LUT is not present");
        return {std::move(spContainer)};
    }

    auto customControls = CreateCustomControls();

    auto resetCB = [this] (uint32_t clientID) -> void
    {
        _spGamma->UpdateValue(GAMMA_DEFAULT_VALUE, true);
        _spEnableLayer->UpdateValue(true, true);
        _spEnableCustomLayer->UpdateValue(false, true);
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetCB, "Reset", "OJL/Images/Reset.png" }
    });

    auto bypassCB = [this] (uint32_t clientID, const bool& value) -> void
    {
        _spLut1d->SetBypass(value);

        WorkOutWhichElementsToEnable();
    };

    _spBypass = spContainer->AddBoolControl("Bypass", bypassCB, "lut1dBypass", _bypassByDefault);

    spContainer->AddSeparator();

    // Generation Controls

    auto enableLayerCB = [this] (uint32_t clientID, bool& value) -> void
    {
        _spLut1d->EnableLayer(GAMMA_LAYER_IDX, value);

        _gammaLayerEnabled = value;

        WorkOutWhichElementsToEnable();

        if (_ready)
            UploadLuts(clientID);
    };

    auto curveTypeCB = [this] (uint32_t clientID, const UiEnumOption& selected, uint32_t controlUserData) -> void
    {
        _curve = (TransferFunctionCurve)selected._userItemData;
        _spLut1d->SetTransferCurve(GAMMA_LAYER_IDX, (TransferFunctionCurve)_curve);

        if (_ready)
        {
            if (_curve == TransferFunctionCurve::Custom && _knots.size() == 0)
            {
                ResetCurve();
            }

            WorkOutWhichElementsToEnable();

            UploadLuts(clientID);
        }
    };

    auto transferFuncCB = [this] (uint32_t clientID, const UiEnumOption& selected, uint32_t controlUserData) -> void
    {
        _type = (TransferFunctionType)selected._userItemData;
        _spLut1d->SetTransferFunction(GAMMA_LAYER_IDX, (TransferFunctionType)selected._userItemData);

        if (_ready)
        {
            WorkOutWhichElementsToEnable();

            UploadLuts(clientID);
        }
    };

    auto gammaCB = [this] (uint32_t clientID, const float& value) -> void
    {
        _spLut1d->SetGamma(GAMMA_LAYER_IDX, value);

        UploadLuts(clientID);
    };

    auto blackLumCB = [this] (uint32_t clientID, const float& value) -> void
    {
        _spLut1d->SetBlackLum(GAMMA_LAYER_IDX, value);

        UploadLuts(clientID);
    };

    auto whiteLumCB = [this] (uint32_t clientID, const float& value) -> void
    {
        _spLut1d->SetWhiteLum(GAMMA_LAYER_IDX, value);

        UploadLuts(clientID);
    };

    _spEnableLayer = spContainer->AddBoolControl("Enable Gamma Layer", enableLayerCB, "EnableLayer", true);
    _spCurveType = spContainer->AddEnumControl("Curve Type", _curveOptions, curveTypeCB, "CurveType", (uint32_t)TransferFunctionCurve::Gamma, 0);
    _spTransferFunc = spContainer->AddEnumControl("Transfer Function", _funcTypeOptions, transferFuncCB, "FunctionType", (uint32_t)TransferFunctionType::OETF, 0);
    _spGamma = spContainer->AddSliderControl("Gamma", 0.0f, 30.0f, gammaCB, "Gamma", GAMMA_DEFAULT_VALUE);
    _spBlackLum = spContainer->AddSliderControl("Black Luminance", 0.0f, 1024.0f, blackLumCB, "BlackLuminance", 10.0f);
    _spWhiteLum = spContainer->AddSliderControl("White Luminance", 0.0f, 1024.0f, whiteLumCB, "WhiteLuminance", 10.0f);

    spContainer->AddSeparator();

    auto enableCustomLayerCB = [this] (uint32_t clientID, bool& value) -> void
    {
        _spLut1d->EnableLayer(CUSTOM_LAYER_IDX, value);

        _customLayerEnabled = value;

        WorkOutWhichElementsToEnable();

        if (_ready)
            UploadLuts(clientID);
    };

    _spEnableCustomLayer = spContainer->AddBoolControl("Enable Custom Curve", enableCustomLayerCB, "EnableCustomLayer", false);

    auto createCurveCB = [this, customControls = std::move(customControls)] (uint32_t clientID) -> void
    {
        auto closedCB = [this] (uint32_t clientID, bool ok) -> void
        {
            // Set knots and control points
            _spLut1d->SetKnots(CUSTOM_LAYER_IDX, _knots);
            _spLut1d->SetControlPoints(CUSTOM_LAYER_IDX, _controlPoints);
        };

        Show1dLutDialogUiUpdate(customControls, clientID, closedCB, "Custom Curve Creation");

        if(_spHs)
            UpdateRoi(true, true);

        _spCustomCurve->UpdateValue(GetContainer());
    };

    _spCustomiseCurve = spContainer->AddButtonControl("Customise Curve", createCurveCB);

    spContainer->AddSeparator();

    if (_enableDebugUi)
    {
        spContainer->AddSeparator();
        _spFrameStats = spContainer->AddLabelControl("Checksum:", "");
    }

    _ready = true;

    WorkOutWhichElementsToEnable();

    UploadLuts(0);

    if(_spTmo)
        _spTmo->ReleaseOverride();

    _spContainer = spContainer;

    return {std::move(spContainer)};
}


void Lut1dControls::Enable(bool v)
{
    if(_spContainer)
    {
        _spContainer->SetEnabled(v);

        if(v)
        {
            WorkOutWhichElementsToEnable();
        }
    }
}


void Lut1dControls::StatsUpdateLoop()
{
    if (_enableDebugUi)
    {
        uint32_t stats;
        _spLut1d->GetFrameStats(&stats);
        _spFrameStats->UpdateValue(std::to_string(stats));
    }
}

///////////////////////////////////////////////////////////////////////////////

Show1dLutDialogUiUpdate::Show1dLutDialogUiUpdate(
                                            const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                                            uint32_t clientID, DialogClosedCB closedCB, std::string title)
    // The dialog type, message don't matter because we're overwriting the jsonString
:   ModalDialogUiUpdate(clientID, DialogType::MESSAGE, "", closedCB, closedCB)
{

    UiUpdateJSON document;
    AtUtils::IJsonObjectPtr spNode = document.GetRoot();
    AtUtils::IJsonObjectPtr spCommandNode = spNode->AddObject("ModalDialog");
    spCommandNode->AddValue("dialog_type", "ISPAWBDialog");
    spCommandNode->AddValue("dialog_title", title);

    auto spControlsArray = spCommandNode->AddArray("Controls");

    for (const auto& spControl : controls)
    {
        auto spJsonObject = spControlsArray->AddElement()->AddObject();
        spControl->GetJSON(spJsonObject);
    }

    _jsonString = document.ToString();
    Notify();
}

///////////////////////////////////////////////////////////////////////

CustomCurveInfo CustomCurveInfo::FromString(bool csv, const std::string& strValue)
{
    CustomCurveInfo value{0};

    // std::cout << strValue << std::endl;

    if (csv)
    {
        std::string stringValue(strValue);

        // std::string numBarsStr = UiElement::CsvExtract(stringValue);
        // value.num_bars = AtUtils::FromString<uint32_t>(numBarsStr);

        // value.names[0] = UiElement::CsvExtract(stringValue);
        // for (int i = 0; i < value.num_bars; i++)
        // {
        //     std::string valString = UiElement::CsvExtract(stringValue);
        //     value.values[0][i] = AtUtils::FromString<uint32_t>(valString);
        // }

        // value.names[1] = UiElement::CsvExtract(stringValue);
        // for (int i = 0; i < value.num_bars; i++)
        // {
        //     std::string valString = UiElement::CsvExtract(stringValue);
        //     value.values[1][i] = AtUtils::FromString<uint32_t>(valString);
        // }

        std::string num_pts_str = UiElement::CsvExtract(stringValue);
        value.num_segments = AtUtils::FromString<uint32_t>(num_pts_str);
        for (int i = 0; i < value.num_segments; i++)
        {
            std::string x_str = UiElement::CsvExtract(stringValue);
            std::string y_str = UiElement::CsvExtract(stringValue);

            value.control_points[i][0] = AtUtils::FromString<double>(x_str);
            value.control_points[i][1] = AtUtils::FromString<double>(y_str);
        }

        for (int i = 0; i < value.num_segments + 1; i++)
        {
            std::string x_str = UiElement::CsvExtract(stringValue);
            std::string y_str = UiElement::CsvExtract(stringValue);

            value.knots[i][0] = AtUtils::FromString<double>(x_str);
            value.knots[i][1] = AtUtils::FromString<double>(y_str);
        }
    }
    else
    {
        auto spJson = AtUtils::IJson::Create(strValue);
        if (!spJson)
            return {};

        auto spObject = spJson->Parse();
        if (!spObject)
            return {};

        value.num_bars = static_cast<uint32_t>(spObject->GetValue<uint32_t>("num_bars"));

        AtUtils::IJsonArrayPtr valueArray0 = spObject->GetArray("values0");
        AtUtils::IJsonArrayPtr valueArray1 = spObject->GetArray("values1");
        for (int i = 0; i < (int)valueArray0->Size(); i++)
        {
            value.values[0][i] = std::get<uint32_t>(valueArray0->At(i)->GetValue());
            value.values[1][i] = std::get<uint32_t>(valueArray1->At(i)->GetValue());
        }

        value.names[0] = static_cast<std::string>(spObject->GetValue<std::string>("name0"));
        value.names[1] = static_cast<std::string>(spObject->GetValue<std::string>("name1"));

        value.num_segments = static_cast<double>(spObject->GetValue<uint32_t>("num_control_points"));

        AtUtils::IJsonArrayPtr pointsArray = spObject->GetArray("control_points");

        if(pointsArray)
        {
            for (int i = 0; i < value.num_segments; i++)
            {
                auto pointArray = pointsArray->At(i)->GetArray();

                if(pointArray)
                {
                    value.control_points[i][0] = std::get<double>(pointArray->At(0)->GetValue());
                    value.control_points[i][1] = std::get<double>(pointArray->At(1)->GetValue());
                }
            }
        }

        AtUtils::IJsonArrayPtr knotsArray = spObject->GetArray("knots");

        if(knotsArray)
        {
            for (int i = 0; i < value.num_segments + 1; i++)
            {
                auto knotArray = knotsArray->At(i)->GetArray();

                if(knotArray)
                {
                    value.knots[i][0] = std::get<double>(knotArray->At(0)->GetValue());
                    value.knots[i][1] = std::get<double>(knotArray->At(1)->GetValue());
                }
            }
        }
    }

    return value;
}
void CustomCurveInfo::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("num_bars", num_bars);
    auto spValues0Array = spJsonObject->AddArray("values0");
    auto spValues1Array = spJsonObject->AddArray("values1");

    if(spValues0Array && spValues1Array)
    {
        for (int i = 0; i < num_bars; i++)
        {
            auto spJsonObject0 = spValues0Array->AddElement();
            spJsonObject0->AddValue(values[0][i]);

            auto spJsonObject1 = spValues1Array->AddElement();
            spJsonObject1->AddValue(values[1][i]);
        }
    }

    spJsonObject->AddValue("name0", names[0]);
    spJsonObject->AddValue("name1", names[1]);

    spJsonObject->AddValue("num_control_points", num_segments);

    auto spPointsArray = spJsonObject->AddArray("control_points");
    for (int i = 0; i < num_segments; i++)
    {
        auto spPointArray = spPointsArray->AddElement()->AddArray();

        spPointArray->AddElement()->AddValue(control_points[i][0]);
        spPointArray->AddElement()->AddValue(control_points[i][1]);
    }


    auto spKnotsArray = spJsonObject->AddArray("knots");
    for (int i = 0; i < num_segments + 1; i++)
    {
        auto spKnotArray = spKnotsArray->AddElement()->AddArray();

        spKnotArray->AddElement()->AddValue(knots[i][0]);
        spKnotArray->AddElement()->AddValue(knots[i][1]);
    }
}
std::string CustomCurveInfo::ToString(bool csv)
{
    if (csv)
    {
		std::string stringValue = AtUtils::FormatString("%d%s,%d%s", num_bars,
                                                            ValuesString().c_str(), num_segments, PointsString().c_str());
		return stringValue;
    }
    else
    {
        auto spJson = AtUtils::IJson::Create();
        if (!spJson)
            return {};

        auto spObject = spJson->RootObject();
        if (!spObject)
            return {};

        spObject->AddValue("num_bars", num_bars);
        auto spValues0Array = spObject->AddArray("values0");
        auto spValues1Array = spObject->AddArray("values1");

        for (int i = 0; i < num_bars; i++)
        {
            auto spJsonObject0 = spValues0Array->AddElement();
            spJsonObject0->AddValue(values[0][i]);

            auto spJsonObject1 = spValues1Array->AddElement();
            spJsonObject1->AddValue(values[1][i]);
        }
        spObject->AddValue("names0", names[0]);
        spObject->AddValue("names1", names[1]);

        spObject->AddValue("num_control_points", num_segments);

        auto spPoints = spObject->AddArray("control_points");
        for (int i = 0; i < num_segments; i++)
        {
            auto spJsonObject = spPoints->AddElement();
            auto spPoint = spJsonObject->AddArray();

            auto spX = spPoint->AddElement();
            spX->AddValue(control_points[i][0]);
            auto spY = spPoint->AddElement();
            spY->AddValue(control_points[i][1]);
        }

        auto spKnots = spObject->AddArray("knots");
        for (int i = 0; i < num_segments + 1; i++)
        {
            auto spJsonObject = spKnots->AddElement();
            auto spKnot = spJsonObject->AddArray();

            auto spX = spKnot->AddElement();
            spX->AddValue(knots[i][0]);
            auto spY = spKnot->AddElement();
            spY->AddValue(knots[i][1]);
        }

        return spJson->ToString();
    }
}

std::string CustomCurveInfo::ValuesString()
{
    std::stringstream ss;
    for (int i = 0; i < 2; i++)
    {
        ss << "," << names[i];
        for (int j = 0; j < num_bars; j++)
        {
            ss << "," << values[i][j];
        }
    }

    return ss.str();
}

std::string CustomCurveInfo::PointsString()
{
    std::stringstream ss;
    for (int i = 0; i < num_segments; i++)
    {
        ss << "," << control_points[i][0] << "," << control_points[i][1];
    }
    for (int i = 0; i < num_segments + 1; i++)
    {
        ss << "," << knots[i][0] << "," << knots[i][1];
    }

    return ss.str();
}

//////////////////////////////////////////

std::string Lut1dRoI::ToString(bool csvFormat)
{
    if (csvFormat)
    {
		std::string stringValue = AtUtils::FormatString("%f,%f,%f,%f,%s,%s,%s", _x, _y,
                                                        _width, _height,
                                                        _enabled ? "true" : "false",
                                                        _outside ? "true" : "false",
                                                        _tmo_enabled ? "true" : "false");
		return stringValue;
    }
    else
    {
        auto spJson = AtUtils::IJson::Create();
        if (!spJson)
            return {};

        auto spObject = spJson->RootObject();
        if (!spObject)
            return {};

        spObject->AddValue("x", _x);
        spObject->AddValue("y", _y);
        spObject->AddValue("width", _width);
        spObject->AddValue("height", _height);
        spObject->AddValue("enabled", _enabled);
        spObject->AddValue("outside", _outside);
        spObject->AddValue("tmo_enabled", _tmo_enabled);
        return spJson->ToString();
    }
}

void Lut1dRoI::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("x", _x);
    spJsonObject->AddValue("y", _y);
    spJsonObject->AddValue("width", _width);
    spJsonObject->AddValue("height", _height);
    spJsonObject->AddValue("enabled", _enabled);
    spJsonObject->AddValue("outside", _outside);
    spJsonObject->AddValue("tmo_enabled", _tmo_enabled);
}

Lut1dRoI Lut1dRoI::FromString(bool csvFormat, const std::string& strValue)
{
    Lut1dRoI value{0};

    if (csvFormat)
    {
        std::string stringValue(strValue);
        std::string xString = UiElement::CsvExtract(stringValue);
        std::string yString = UiElement::CsvExtract(stringValue);
        std::string widthString = UiElement::CsvExtract(stringValue);
        std::string heightString = UiElement::CsvExtract(stringValue);
        std::string enabledString = UiElement::CsvExtract(stringValue);
        std::string outsideString = UiElement::CsvExtract(stringValue);
        std::string tmoEnabledString = UiElement::CsvExtract(stringValue);

        value._x = AtUtils::FromString<float>(xString);
        value._y = AtUtils::FromString<float>(yString);
        value._width = AtUtils::FromString<float>(widthString);
        value._height = AtUtils::FromString<float>(heightString);
        value._enabled = AtUtils::FromString<bool>(enabledString);
        value._outside = AtUtils::FromString<bool>(outsideString);
        value._tmo_enabled = AtUtils::FromString<bool>(tmoEnabledString);
    }
    else
    {
        auto spJson = AtUtils::IJson::Create(strValue);
        if (!spJson)
            return {};

        auto spObject = spJson->Parse();
        if (!spObject)
            return {};

        value._x = static_cast<float>(spObject->GetValue<double>("x"));
        value._y = static_cast<float>(spObject->GetValue<double>("y"));
        value._width = static_cast<float>(spObject->GetValue<double>("width"));
        value._height = static_cast<float>(spObject->GetValue<double>("height"));
        value._enabled = spObject->GetValue<bool>("enabled");
        value._outside = spObject->GetValue<bool>("outside");
        value._tmo_enabled = spObject->GetValue<bool>("tmo_enabled");
    }

    return value;
}