/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "BlsControls.h"
#include "UiElements.h"

#include <cmath>

using namespace SwApi;

static const uint32_t margin = 12;
static const uint32_t default_panel_width = 450;

static const std::string redString = "#FF1A1A";
static const std::string greenString = "#33FF33";
static const std::string blueString ="#0033FF";

BlsControls::BlsControls(const std::shared_ptr<SwApi::Bls>& spBls, bool enableDebugUi)
    : _spBls(spBls)
    , _enableDebugUi(enableDebugUi)
{
}


std::vector<std::shared_ptr<UiControlContainer>> BlsControls::AddUiElements()
{

    auto spContainer = std::make_shared<UiControlContainer>("Black Level Statistics", GetSettingsSectionName());
    if (!_spBls)
    {
        spContainer->AddLabelControl("BLS not present");
        return { std::move(spContainer) };
    }

    spContainer->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin});
    spContainer->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    spContainer->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 350 });
    spContainer->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 525 });

    auto graphUpdateCB = [this](uint32_t clientId, const BLSChartInfo& value)
    {
        //This shouldn't ever trigger
        std::cout << "Why are we here?\n";
    };

    BLSChartInfo container = GetContainer();

    _spGraphCustomControl = std::make_shared<GraphCustomControl>("OJISPBLSChart", container, graphUpdateCB);
    spContainer->Add(_spGraphCustomControl);

    return { std::move(spContainer) };
}

void BlsControls::PopulateMinMaxValues(BLSChartInfo& info, uint32_t target)
{
    int32_t biggestDiff = 0;
    bool allMatchTarget = true;

    for (int i = 0; i < 4; i++)
    {
        int absDiff = abs(target - info.values[i]);
        if (absDiff > biggestDiff)
        {
            biggestDiff = absDiff;
        }
        allMatchTarget &= (target == info.values[i]);
    }

    if (allMatchTarget)
    {

        info.max_value = 2*target;
        info.min_value = 0;
    }
    else
    {
        info.max_value = target + 2*biggestDiff;
        info.min_value = target - 2*biggestDiff;
        if (info.min_value < 0)
        {
            info.min_value = 0;
        }
    }
}

void BlsControls::UpdateGraphValues(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t target)
{
    BLSChartInfo container;

    container.chart_names[0] = "cfa 00";
    container.chart_names[1] = "cfa 01";
    container.chart_names[2] = "cfa 10";
    container.chart_names[3] = "cfa 11";

    container.values[0] = x0;
    container.values[1] = x1;
    container.values[2] = x2;
    container.values[3] = x3;

    container.update_colours = true;

    switch (_spBls->GetCfaPhase())
    {
        case TCfaPhase::RGGB: // RGGB
        {
            container.chart_colours[0] = redString;
            container.chart_colours[1] = greenString;
            container.chart_colours[2] = greenString;
            container.chart_colours[3] = blueString;
            break;
        }
        case TCfaPhase::GRBG: //GRBG
        {
            container.chart_colours[0] = greenString;
            container.chart_colours[1] = redString;
            container.chart_colours[2] = blueString;
            container.chart_colours[3] = greenString;
            break;
        }
        case TCfaPhase::GBRG: // GBRG
        {
            container.chart_colours[0] = greenString;
            container.chart_colours[1] = blueString;
            container.chart_colours[2] = redString;
            container.chart_colours[3] = greenString;
            break;
        }
        case TCfaPhase::BGGR: // BGGR
        {
            container.chart_colours[0] = blueString;
            container.chart_colours[1] = greenString;
            container.chart_colours[2] = greenString;
            container.chart_colours[3] = redString;
            break;
        }
    }

    PopulateMinMaxValues(container, target);

    _spGraphCustomControl->UpdateValue(container);
}

BLSChartInfo BlsControls::GetContainer()
{
    BLSChartInfo container;
    container.chart_names[0] = "cfa 00";
    container.chart_names[1] = "cfa 01";
    container.chart_names[2] = "cfa 10";
    container.chart_names[3] = "cfa 11";

    uint32_t dump_store;

    _spBls->GetPedestalsAndScalarsForCurrentConfig(
        &container.values[0],
        &container.values[1],
        &container.values[2],
        &container.values[3],
        &dump_store,
        &dump_store,
        &dump_store,
        &dump_store);

    container.update_colours = true;

    switch (_spBls->GetCfaPhase())
    {
        case TCfaPhase::RGGB: // RGGB
        {
            container.chart_colours[0] = redString;
            container.chart_colours[1] = greenString;
            container.chart_colours[2] = greenString;
            container.chart_colours[3] = blueString;
            break;
        }
        case TCfaPhase::GRBG: //GRBG
        {
            container.chart_colours[0] = greenString;
            container.chart_colours[1] = redString;
            container.chart_colours[2] = blueString;
            container.chart_colours[3] = greenString;
            break;
        }
        case TCfaPhase::GBRG: // GBRG
        {
            container.chart_colours[0] = greenString;
            container.chart_colours[1] = blueString;
            container.chart_colours[2] = redString;
            container.chart_colours[3] = greenString;
            break;
        }
        case TCfaPhase::BGGR: // BGGR
        {
            container.chart_colours[0] = blueString;
            container.chart_colours[1] = greenString;
            container.chart_colours[2] = greenString;
            container.chart_colours[3] = redString;
            break;
        }
    }

    int average = (container.values[0] + container.values[1] + container.values[2] + container.values[3]) / 4;

    PopulateMinMaxValues(container, average);

    return container;
}


///////// BLSBLSChartInfo

BLSChartInfo BLSChartInfo::FromString(bool csv, const std::string& strValue)
{
    BLSChartInfo value{0};

    if (csv)
    {
        std::string stringValue(strValue);

        std::string updateColoursStr = UiElement::CsvExtract(stringValue);
        value.update_colours = AtUtils::FromString<bool>(updateColoursStr);
        std::string maxValStr = UiElement::CsvExtract(stringValue);
        value.max_value = AtUtils::FromString<uint32_t>(maxValStr);
        std::string minValStr = UiElement::CsvExtract(stringValue);
        value.min_value = AtUtils::FromString<uint32_t>(minValStr);

        for (int i = 0; i < 4; i++)
        {
            value.chart_names[i] = UiElement::CsvExtract(stringValue);
            std::string valString = UiElement::CsvExtract(stringValue);
            value.values[i] = AtUtils::FromString<uint32_t>(valString);
            value.chart_colours[i] = UiElement::CsvExtract(stringValue);
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

        value.update_colours = static_cast<bool>(spObject->GetValue<bool>("update_colours"));
        value.max_value = static_cast<uint32_t>(spObject->GetValue<uint32_t>("max_value"));
        value.min_value = static_cast<uint32_t>(spObject->GetValue<uint32_t>("min_value"));

        AtUtils::IJsonArrayPtr valueArray = spObject->GetArray("values");
        AtUtils::IJsonArrayPtr chartNameArray = spObject->GetArray("chart_names");
        AtUtils::IJsonArrayPtr chartColourArray = spObject->GetArray("chart_colours");

        for (int i = 0; i < (int)valueArray->Size(); i++)
        {
            value.chart_names[i] = std::get<std::string>(chartNameArray->At(i)->GetValue());
            value.values[i] = std::get<uint32_t>(valueArray->At(i)->GetValue());
            value.chart_colours[i] = std::get<std::string>(chartColourArray->At(i)->GetValue());
        }
    }

    return value;
}
void BLSChartInfo::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("update_colours", update_colours);
    spJsonObject->AddValue("max_value", max_value);
    spJsonObject->AddValue("min_value", min_value);
    auto spValueArray = spJsonObject->AddArray("values");
    auto spChartNameArray = spJsonObject->AddArray("chart_names");
    auto spChartColourArray = spJsonObject->AddArray("chart_colours");

    for (int i = 0; i < 4; i++)
    {
        auto spJsonValue = spValueArray->AddElement();
        spJsonValue->AddValue(values[i]);

        auto spJsonName = spChartNameArray->AddElement();
        spJsonName->AddValue(chart_names[i]);

        auto spJsonColour = spChartColourArray->AddElement();
        spJsonColour->AddValue(chart_colours[i]);
    }
}
std::string BLSChartInfo::ToString(bool csv)
{
    if (csv)
    {
		std::string stringValue = AtUtils::FormatString("%s,%d,%d%s",
            update_colours ? ("True") : ("False"),
            max_value, min_value,ValuesString().c_str());
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
        spObject->AddValue("update_colours", update_colours);
        spObject->AddValue("max_value", max_value);
        spObject->AddValue("min_value", min_value);
        auto spValueArray = spObject->AddArray("values");
        auto spChartNameArray = spObject->AddArray("chart_names");
        auto spChartColourArray = spObject->AddArray("chart_colours");

        for (int i = 0; i < 4; i++)
        {
            auto spJsonValue = spValueArray->AddElement();
            spJsonValue->AddValue(values[i]);

            auto spJsonName = spChartNameArray->AddElement();
            spJsonName->AddValue(chart_names[i]);

            auto spJsonColour = spChartColourArray->AddElement();
            spJsonColour->AddValue(chart_colours[i]);
        }

        return spJson->ToString();
    }
}

std::string BLSChartInfo::ValuesString()
{
    std::stringstream ss;
    for (int i = 0; i < 4; i++)
    {
        ss << "," << chart_names[i];
        ss << "," << values[i];
        ss << "," << chart_colours[i];
    }

    return ss.str();
}