/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "HsControls.h"
#include "UiElements.h"
#include <sstream>
#include <cmath>

using namespace SwApi;


HsControls::HsControls(const std::string& name, 
                        const std::shared_ptr<SwApi::HistogramStats>& spHs,
                        const IROIPtr& spIROI,
                        bool enableDebugUi)
    : _name(name)
    , _spHs(spHs)
    , _spIROI(spIROI)
    , _displayType(DisplayType::Frame)
    , _enableDebugUi(enableDebugUi)
{
    _roi._enabled = false;
    _roi._outside = false;
    _roi._tmo_enabled = false;
    _roi._x = 0.0f;
    _roi._y = 0.0f;
    _roi._width = 1.0f;
    _roi._height = 1.0f;

    if (_spHs)
    {
        _histogramSubscriptionId = _spHs->RegisterHistogramObserver([this](const std::shared_ptr<SwApi::Hs::TableResults>& histogramData)
        {
            std::scoped_lock lock(_histogramDataMutex);
            _spLatestHistogramData = histogramData;
        });
    }
}

HsControls::~HsControls()
{
    if (_spHs && _histogramSubscriptionId != 0)
    {
        _spHs->UnregisterHistogramObserver(_histogramSubscriptionId);
    }
}

std::vector<std::shared_ptr<UiControlContainer>> HsControls::AddUiElements()
{
    static constexpr uint32_t PANEL_WIDTH = 450;
    static constexpr uint32_t MARGIN = 12;

    auto spContainer = std::make_shared<UiControlContainer>(_name,
                                                            GetSettingsSectionName());
    if (not _spHs)
    {
        spContainer->AddLabelControl("Histogram Statistics is not present");
        return {std::move(spContainer)};
    }

    //FIXME - calculate this nicer somehow
    spContainer->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN * 3 + (PANEL_WIDTH + MARGIN) * 2 }); // 2 Panels to the left, so 2 widths + 2 margins
    spContainer->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    spContainer->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1200 + 2 * MARGIN });
    spContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 760 });

    _spChartPanel = std::make_shared<UiControlContainer>("Histogram", GetSettingsSectionName());
	_spStatsUpdatePanel = std::make_shared<UiControlContainer>("Stats Updates", GetSettingsSectionName()+"_Updates");
    _spStatsConfigPanel = std::make_shared<UiControlContainer>("Stats Config", GetSettingsSectionName()+"_Config");

    spContainer->Add(_spChartPanel);
    spContainer->Add(_spStatsUpdatePanel);
    spContainer->Add(_spStatsConfigPanel);

    _spRoiPanel = std::make_shared<UiControlContainer>("Region of Interest ", GetSettingsSectionName()+"_Roi");
    _spRoiEditorPanel = std::make_shared<UiControlContainer>("RoI Editor", GetSettingsSectionName()+"_RoiEditor");
    _spRoiXYPanel = std::make_shared<UiControlContainer>("Region of Interest", GetSettingsSectionName()+"_RoiSettings");
    _spRoiWHPanel = std::make_shared<UiControlContainer>("Region of Interest", GetSettingsSectionName()+"_RoiSettings");
    _spRoiHighlightPanel = std::make_shared<UiControlContainer>("Region of Interest", GetSettingsSectionName()+"_RoiHighlight");

    _spRoiPanel->Add(_spRoiEditorPanel);
    _spRoiPanel->Add(_spRoiXYPanel);
    _spRoiPanel->Add(_spRoiWHPanel);
    _spRoiPanel->Add(_spRoiHighlightPanel);

    _spChartPanel->EnableBorder(false);
    _spChartPanel->SetSpacing(0);
    _spChartPanel->SetTransparent(true);

    _spStatsUpdatePanel->EnableBorder(false);
    _spStatsUpdatePanel->SetSpacing(0);
    _spStatsUpdatePanel->SetTransparent(true);

    _spStatsConfigPanel->EnableBorder(false);
    _spStatsConfigPanel->SetSpacing(0);
    _spStatsConfigPanel->SetTransparent(true);

    auto graphUpdateCB = [this](uint32_t clientId, const ChartInfo& value){};

    _spGraphCustomControl = std::make_shared<GraphCustomControl>("OJBarChartControl", ChartInfo{}, graphUpdateCB);

    _spChartPanel->Add(_spGraphCustomControl);

    auto toggleAutoCB = [this](uint32_t clientID, bool value)
    {
        if (_fpHsAutoControlToggleCb)
        {
            _fpHsAutoControlToggleCb(value);
        }
    };

    _spStatsUpdatePanel->AddBoolControl("Auto Update", toggleAutoCB, "toggleAuto", true);
    
    auto hsUpdateUIStatsCB = [this](uint32_t clientID) -> void
    {
        AutoUpdateCallback();
    };
    
    _spStatsUpdatePanel->AddButtonControl("Read stats", hsUpdateUIStatsCB);

    auto displayModeCB = [this](uint32_t clientID, const UiEnumOption& value, uint32_t) -> void
    {
        _displayType = (DisplayType)value._userItemData;
    };
    
    _spDisplayType = _spStatsConfigPanel->AddEnumControl("Display mode: ", _displayOptions, displayModeCB, "displayMode", DisplayType::Frame);
    
    auto editRoiCB = [this](uint32_t clientID)
    {

        auto windowClosedCB = [this](uint32_t clientID, bool ok) {
            if(_highlighting)
            {
                _highlighting = false;
                _spIROI->SetEnableRoi(_highlighting);
                _spHighlightRoi->UpdateValue(false);
            }
        };

        ShowRoiDialogUiUpdate({ _spRoiPanel }, clientID, windowClosedCB);

        UpdateRoi(true, true);
    };
    
    _spStatsConfigPanel->AddButtonControl("Edit RoI", editRoiCB);

    // Roi Editor Popup

    auto roiResetCB = [this] (uint32_t clientID) -> void
    {
        _roi._x = 0.0f;
        _roi._y = 0.0f;
        _roi._width = 1.0f;
        _roi._height = 1.0f;

        UpdateRoi(true, false);
    };

    _spRoiPanel->AddHeaderButtons( {
        { "RoiReset", roiResetCB, "Reset", "OJL/Images/Reset.png" }
    });

    _spRoiEditorPanel->EnableBorder(false);
    _spRoiEditorPanel->SetSpacing(0);
    _spRoiEditorPanel->SetTransparent(true);

    _spRoiXYPanel->EnableBorder(false);
    _spRoiXYPanel->SetSpacing(0);
    _spRoiXYPanel->SetTransparent(true);

    _spRoiWHPanel->EnableBorder(false);
    _spRoiWHPanel->SetSpacing(0);
    _spRoiWHPanel->SetTransparent(true);

    _spRoiHighlightPanel->EnableBorder(false);
    _spRoiHighlightPanel->SetSpacing(0);
    _spRoiHighlightPanel->SetTransparent(true);

    _spRoiEditorPanel->AddLabelControl("WARNING: Changing the RoI will affect Autoexposure");
    auto xCB = [this](uint32_t clientId, float& value)
    {
        _roi._x = value;

        UpdateRoi(true, false);
    };
    _spX = _spRoiXYPanel->AddFloatControl("X", 0.0f, 0.875, xCB, "roiX", 0.0625f);
    auto yCB = [this](uint32_t clientId, float& value)
    {
        _roi._y = value;

        UpdateRoi(true, false);
    };
    _spY = _spRoiXYPanel->AddFloatControl("Y", 0.0f, 0.7778f, yCB, "roiY", 0.1111f );
    auto widthCB = [this](uint32_t clientId, float& value)
    {
        _roi._width = value;

        UpdateRoi(true,false );
    };
    _spWidth = _spRoiWHPanel->AddFloatControl("Width", 0.0625f, 1.0f, widthCB, "roiWidth", 0.75f);
    auto heightCB = [this](uint32_t clientId, float& value)
    {
        _roi._height = value;

        UpdateRoi(true, false);
    };
    _spHeight = _spRoiWHPanel->AddFloatControl("Height", 0.1111f, 1.0f, heightCB, "roiHeight", 0.5555f);
    auto highlightCB = [this](uint32_t clientID, bool& value)
    {
        if(_spIROI)
        {
            _highlighting = value;
            _spIROI->SetEnableRoi(_highlighting);
        }

        UpdateRoi(false, true);
    };
    _spHighlightRoi = _spRoiHighlightPanel->AddBoolControl("Highlight RoI", highlightCB, "roiHighlight", false);

    auto roiUpdateCB = [this](uint32_t clientId, RoiSelectorData& value)
    {
        _roi._x = value._x;
        _roi._y = value._y;
        _roi._width = value._width;
        _roi._height = value._height;

        _spX->UpdateValue(_roi._x);
        _spY->UpdateValue(_roi._y);
        _spWidth->UpdateValue(_roi._width);
        _spHeight->UpdateValue(_roi._height);

        // To avoid lagging UI only highlight new ROI here
        // Actual HW update will be done in AutoUpdateCallback
        _pendingHwUpdate = true;

        if (_highlighting)
        {
            altera_vvp_roi tmo_roi = {
                ._x = _roi._x,
                ._y = _roi._y,
                ._width = _roi._width,
                ._height = _roi._height
            };
            _spIROI->SetRegionOfInterest(tmo_roi);
        }
    };

    _spRoiCustomControl = std::make_shared<RoiCustomControl>("OJRoiControl", _roi, roiUpdateCB);
    _spRoiEditorPanel->Add(_spRoiCustomControl);

    _spChartPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    _spChartPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    _spChartPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1200 });
    _spChartPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 700 });

    _spStatsUpdatePanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN });
    _spStatsUpdatePanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 600 + MARGIN });
    _spStatsUpdatePanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 320 });
    _spStatsUpdatePanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    _spStatsConfigPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 320 + MARGIN });
    _spStatsConfigPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 600 + MARGIN });
    _spStatsConfigPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 350 });
    _spStatsConfigPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    _spRoiPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN });
    _spRoiPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN });
    _spRoiPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1100 });
    _spRoiPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 725 });

    _spRoiEditorPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN });
    _spRoiEditorPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    _spRoiEditorPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1000 });
    _spRoiEditorPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 585 });

    _spRoiXYPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, MARGIN }); // Match roi editor start
    _spRoiXYPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 590 + MARGIN });
    _spRoiXYPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 320 });
    _spRoiXYPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    _spRoiWHPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 320 + MARGIN });
    _spRoiWHPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 590 + MARGIN });
    _spRoiWHPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 320 });
    _spRoiWHPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    _spRoiHighlightPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 320 * 2 + MARGIN });
    _spRoiHighlightPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 590 + MARGIN });
    _spRoiHighlightPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 320 });
    _spRoiHighlightPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    UpdateRoi(true, true);

    _spIROI->SetEnableRoi(false);

    _ready = true;

    return {std::move(spContainer)};
}

void HsControls::AutoUpdateCallback()
{
    if (!_spGraphCustomControl)
    {
        // Sometimes startup can be slow and this can hit seg faults as a result
        return;
    }

    // If ROI updated locally apply new values to HW
    // otherwise read current HW values if they were updated
    // from the Autoexposure control and update the UI
    UpdateRoi(false, !_pendingHwUpdate);
    _pendingHwUpdate = false;

    std::shared_ptr<SwApi::Hs::TableResults> histogramData;

    {
        std::scoped_lock lock(_histogramDataMutex);
        histogramData = _spLatestHistogramData;
    }

    if (!histogramData)
        return;

    if(!histogramData->valid)
        return;

    ChartInfo container = GetContainer(histogramData);

    _spGraphCustomControl->UpdateValue(container);
}

void HsControls::SetAutoUpdateToggleFunc(std::function<void(bool)> fpHsAutoControlToggleCb)
{
    _fpHsAutoControlToggleCb = std::move(fpHsAutoControlToggleCb);
}

ChartInfo HsControls::GetContainer(const std::shared_ptr<SwApi::Hs::TableResults>& histogramData)
{
    ChartInfo container;
    container.names[0] = "Full Frame";
    container.names[1] = "Region of Interest";
    container.num_bars = 0;

    switch (_displayType)
    {
        case DisplayType::Frame:
        {
            container.num_bars = histogramData->frame_luma_bins.num_luma_bins;
            for (int i = 0; i < container.num_bars; i++)
            {
                container.values[0][i] = histogramData->frame_luma_bins.luma_bins[i];
                container.values[1][i] = 0;
            }
            break;
        }
        case DisplayType::Roi:
        {
            container.num_bars = histogramData->roi_luma_bins.num_luma_bins;
            for (int i = 0; i < container.num_bars; i++)
            {
                container.values[0][i] = 0;
                container.values[1][i] = histogramData->roi_luma_bins.luma_bins[i];
            }
            break;
        }
        case DisplayType::Overlay:
        {
            container.num_bars = histogramData->frame_luma_bins.num_luma_bins;
            for (int i = 0; i < container.num_bars; i++)
            {
                container.values[0][i] = histogramData->frame_luma_bins.luma_bins[i];
                container.values[1][i] = histogramData->roi_luma_bins.luma_bins[i];
            }
            break;
        }
    }

    return container;
}


void HsControls::UpdateRoi(bool updateUI, bool readHs)
{
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
            _roi._x = static_cast<float>(pix_roi.h_start) / w;
            _roi._y = static_cast<float>(pix_roi.v_start) / h;
            _roi._width = static_cast<float>(pix_roi.h_end - pix_roi.h_start) / w;
            _roi._height = static_cast<float>(pix_roi.v_end - pix_roi.v_start) / h;

            const constexpr float MIN_ROI_SIZE = 0.0625f;

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
                .h_end = (uint16_t)((_roi._x + _roi._width) * w - 1),
                .v_end = (uint16_t)((_roi._y + _roi._height) * h - 1)
            };

            _spHs->SetRegionOfInterest(pix_roi);
        }

        if (updateUI)
        {
            _spX->UpdateValue(_roi._x);
            _spY->UpdateValue(_roi._y);
            _spWidth->UpdateValue(_roi._width);
            _spHeight->UpdateValue(_roi._height);

            _spRoiCustomControl->UpdateValue(_roi);
        }

        if (_highlighting)
        {
            altera_vvp_roi roi = {
                ._x = _roi._x,
                ._y = _roi._y,
                ._width = _roi._width,
                ._height = _roi._height
            };
            _spIROI->SetRegionOfInterest(roi);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

ShowRoiDialogUiUpdate::ShowRoiDialogUiUpdate(
                                            const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                                            uint32_t clientID, DialogClosedCB closedCB)
    // The dialog type, message don't matter because we're overwriting the jsonString
:   ModalDialogUiUpdate(clientID, DialogType::MESSAGE, "", closedCB, closedCB)
{

    UiUpdateJSON document;
    AtUtils::IJsonObjectPtr spNode = document.GetRoot();
    AtUtils::IJsonObjectPtr spCommandNode = spNode->AddObject("ModalDialog");
    spCommandNode->AddValue("dialog_type", "ISPAWBDialog");
    spCommandNode->AddValue("dialog_title", "Histogram Statistics Region of Interest");
    spCommandNode->AddValue("width", 1150);

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

ChartInfo ChartInfo::FromString(bool csv, const std::string& strValue)
{
    ChartInfo value{0};

    if (csv)
    {
        std::string stringValue(strValue);

        std::string numBarsStr = UiElement::CsvExtract(stringValue);
        value.num_bars = AtUtils::FromString<uint32_t>(numBarsStr);

        value.names[0] = UiElement::CsvExtract(stringValue);;
        for (int i = 0; i < value.num_bars; i++)
        {
            std::string valString = UiElement::CsvExtract(stringValue);
            value.values[0][i] = AtUtils::FromString<uint32_t>(valString);
        }

        value.names[1] = UiElement::CsvExtract(stringValue);;
        for (int i = 0; i < value.num_bars; i++)
        {
            std::string valString = UiElement::CsvExtract(stringValue);
            value.values[1][i] = AtUtils::FromString<uint32_t>(valString);
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
    }

    return value;
}
void ChartInfo::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("num_bars", num_bars);
    auto spValues0Array = spJsonObject->AddArray("values0");
    auto spValues1Array = spJsonObject->AddArray("values1");

    for (int i = 0; i < num_bars; i++)
    {
        auto spJsonObject0 = spValues0Array->AddElement();
        spJsonObject0->AddValue(values[0][i]);

        auto spJsonObject1 = spValues1Array->AddElement();
        spJsonObject1->AddValue(values[1][i]);
    }

    spJsonObject->AddValue("name0", names[0]);
    spJsonObject->AddValue("name1", names[1]);
}
std::string ChartInfo::ToString(bool csv)
{
    if (csv)
    {
		std::string stringValue = AtUtils::FormatString("%d%s", num_bars,
                                                            ValuesString().c_str());
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

        return spJson->ToString();
    }
}

std::string ChartInfo::ValuesString()
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
