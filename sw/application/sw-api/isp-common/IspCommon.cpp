/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <iostream>
#include <sstream>
#include "IspCommon.h"
#include "UiElements.h"

const char* ToString(const TCfaPhase& type) 
{
    switch (type) 
    {
        case TCfaPhase::RGGB: 
        {
            return "RGGB";
            break;
        }
        case TCfaPhase::GRBG: 
        {
            return "GRBG";
            break;
        }
        case TCfaPhase::GBRG: 
        {
            return "GBRG";
            break;
        }
        case TCfaPhase::BGGR: 
        {
            return "BGGR";
            break;
        }
        default: 
        {
            return "Invalid";
            break;
        }
    }
};

std::ostream& operator<<(std::ostream& o, const TCfaPhase& type) 
{
    return o << ToString(type);
};


RoiSelectorData::RoiSelectorData():
    _enabled{false},
    _outside{false},
    _tmo_enabled{false},
    _x{0.0f},
    _y{0.0f},
    _width{1.0f},
    _height{1.0f}
{
}


std::string RoiSelectorData::ToString(bool csvFormat)
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

void RoiSelectorData::GetJSON(AtUtils::IJsonObjectPtr& spJsonObject)
{
    spJsonObject->AddValue("x", _x);
    spJsonObject->AddValue("y", _y);
    spJsonObject->AddValue("width", _width);
    spJsonObject->AddValue("height", _height);
    spJsonObject->AddValue("enabled", _enabled);
    spJsonObject->AddValue("outside", _outside);
    spJsonObject->AddValue("tmo_enabled", _tmo_enabled);
}

RoiSelectorData RoiSelectorData::FromString(bool csvFormat, const std::string& strValue)
{
    RoiSelectorData value{};

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
