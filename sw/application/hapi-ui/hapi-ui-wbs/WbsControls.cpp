/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "WbsControls.h"
#include "WbsUtils.h"
#include "UiElements.h"

using namespace SwApi;

#if defined(__linux__)
#include <unistd.h>
#endif

static const uint32_t margin = 12;
static const uint32_t default_panel_width = 450;

WbsControls::WbsControls(const std::string& name,
                         const std::shared_ptr<SwApi::Wbs>& spWbs,
                         bool enableDebugUi, bool powerUser,
                         bool flipDisplay)
    : _name(name)
    , _spWbs(spWbs)
    , _enableDebugUi(enableDebugUi)
    , _powerUser(powerUser)
    , _flipDisplay(flipDisplay)
{
}


std::vector<std::shared_ptr<UiControlContainer>> WbsControls::AddUiElements()
{
    if (not _spWbs)
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>("White Balance Statistics",
                                                            GetSettingsSectionName());

    auto spControlsPanel = std::make_shared<UiControlContainer>("Controls", "WBSControls");

    spContainer->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 2*margin + 350});
    spContainer->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    spContainer->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 574 });
    if (_powerUser || _enableDebugUi)
    {
        spContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 765 });
    }
    else
    {
        spContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 700 });
    }
    //spContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1200 });
    WBSRatioChartInfo container = GetContainer(_flipDisplay);

    auto graphUpdateCB = [this](uint32_t clientId, const WBSRatioChartInfo& value)
    {
        //This shouldn't ever trigger
        std::cout << "Why are we here?\n";
    };

    _spGraphCustomControl = std::make_shared<CustomRatioGraph>("OJISPWBSRatioChart", container, graphUpdateCB);
    spContainer->Add(_spGraphCustomControl);

    spContainer->Add(spControlsPanel);

    spControlsPanel->EnableBorder(false);
    spControlsPanel->SetSpacing(5);
    spControlsPanel->SetTransparent(true);

    auto wbsUpdateCB = [this](uint32_t clientID) -> void
    {
        WBSRatioChartInfo container = GetContainer(_flipDisplay);
        _spGraphCustomControl->UpdateValue(container);
    };

    spControlsPanel->AddButtonControl("Update", wbsUpdateCB);

    if (_enableDebugUi || _powerUser)
    {
        auto boolCB = [this](uint32_t clientID, bool &value) -> void
        {
            if (_setSwitchRouterCB)
            {
                _setSwitchRouterCB(value);
            }
        };
        spControlsPanel->AddBoolControl("WBS State", false, boolCB);
    }


    spControlsPanel->SetLeftAnchor({ANCHOR_TYPE::PARENT_NEAR, margin});
    spControlsPanel->SetTopAnchor({ANCHOR_TYPE::PARENT_NEAR, 565});
    spControlsPanel->SetRightAnchor({ANCHOR_TYPE::FIXED_SIZE, 500});
    spControlsPanel->SetBottomAnchor({ANCHOR_TYPE::FIXED_SIZE, 160});

    return {std::move(spContainer)};
}

WBSRatioChartInfo WbsControls::GetContainer(bool flip)
{
    WBSRatioChartInfo container;

    SwApi::WbsRoI roi;
    roi.h_start = 0;
    roi.v_start = 0;
    const auto [width, height] = _spWbs->GetResolution();
    roi.h_end = (uint16_t)width;
    roi.v_end = (uint16_t)height;
    _spWbs->SetRoI(roi, UpdatePolicy::DeferCommit());
    _spWbs->SetCfaX0Ranges(4.0f, 0.25f, UpdatePolicy::DeferCommit());
    _spWbs->SetCfaX1Ranges(4.0f, 0.25f, UpdatePolicy::Sync());
    WbsResults wbsRead = _spWbs->ReadResultTable();



    int index = 0;

    for (int y = 0; y < 7; y++)
    {
        for (int x = 0; x < 7; x++)
        {
            //std::cout << (y * 7) + x << "\n";
            //std::cout << wbsRead.normalised[y][x] << "\n";
            //std::cout << wbsRead.raw[y][x] << "\n\n";
            // We use 48 here because we're flipping the image using warp.
            // This will invert it horizontally AND vertically
            index = (x + (y * 7));
            if (flip)
            {
                index = 48 - index;
            }
            container.values_r[index] = wbsRead.normalised[y][x].red_strength * 100;
            container.values_g[index] = wbsRead.normalised[y][x].green_strength * 100;
            container.values_b[index] = wbsRead.normalised[y][x].blue_strength * 100;
            container.values_count[index] = wbsRead.raw[y][x].num_pixels_accumulated;
        }
    }
    return container;
}


WBSRatioChartInfo WBSRatioChartInfo::FromString(bool csv, const std::string& strValue)
{
    WBSRatioChartInfo value{0};

    if (csv)
    {
        std::string stringValue(strValue);

        for (int i = 0; i < WBS_NUM_ZONES; i++)
        {
            std::string valString = UiElement::CsvExtract(stringValue);
            value.values_r[i] = AtUtils::FromString<char>(valString);
            valString = UiElement::CsvExtract(stringValue);
            value.values_g[i] = AtUtils::FromString<char>(valString);
            valString = UiElement::CsvExtract(stringValue);
            value.values_b[i] = AtUtils::FromString<char>(valString);
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

        AtUtils::IJsonArrayPtr valueRArray = spObject->GetArray("values_r");
        AtUtils::IJsonArrayPtr valueGArray = spObject->GetArray("values_g");
        AtUtils::IJsonArrayPtr valueBArray = spObject->GetArray("values_b");
        for (int i = 0; i < WBS_NUM_ZONES; i++)
        {
            value.values_r[i] = std::get<uint32_t>(valueRArray->At(i)->GetValue());
            value.values_g[i] = std::get<uint32_t>(valueGArray->At(i)->GetValue());
            value.values_b[i] = std::get<uint32_t>(valueBArray->At(i)->GetValue());
        }
    }

    return value;
}
void WBSRatioChartInfo::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    auto spValueRArray = spJsonObject->AddArray("values_r");
    auto spValueGArray = spJsonObject->AddArray("values_g");
    auto spValueBArray = spJsonObject->AddArray("values_b");

    for (int i = 0; i < WBS_NUM_ZONES; i++)
    {
        auto spJsonValue = spValueRArray->AddElement();
        spJsonValue->AddValue(values_r[i]);
        spJsonValue = spValueGArray->AddElement();
        spJsonValue->AddValue(values_g[i]);
        spJsonValue = spValueBArray->AddElement();
        spJsonValue->AddValue(values_b[i]);
    }
}
std::string WBSRatioChartInfo::ToString(bool csv)
{
    if (csv)
    {
		std::string stringValue = AtUtils::FormatString("%s", ValuesString().c_str());
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

        auto spValueRArray = spObject->AddArray("values_r");
        auto spValueGArray = spObject->AddArray("values_g");
        auto spValueBArray = spObject->AddArray("values_b");

        for (int i = 0; i < WBS_NUM_ZONES; i++)
        {
            auto spJsonValue = spValueRArray->AddElement();
            spJsonValue->AddValue(values_r[i]);
            spJsonValue = spValueGArray->AddElement();
            spJsonValue->AddValue(values_g[i]);
            spJsonValue = spValueBArray->AddElement();
            spJsonValue->AddValue(values_b[i]);
        }

        return spJson->ToString();
    }
}

std::string WBSRatioChartInfo::ValuesString()
{
    std::stringstream ss;
    for (int i = 0; i < WBS_NUM_ZONES; i++)
    {
        if (i > 0)
        {
            ss << ",";
        }
        ss << values_r[i] << ",";
        ss << values_g[i] << ",";
        ss << values_b[i];
    }

    return ss.str();
}