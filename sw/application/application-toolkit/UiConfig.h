/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "AtUtils.h"

class VariantValue : public std::string
{
public:
    VariantValue() {}
    template <typename T>
    VariantValue(const T& value)
    {
        std::string::operator=(AtUtils::ToString(value));
    }
};

using UiConfig = std::unordered_map<std::string, VariantValue>;

class UiConfigUtils
{
public:
    UiConfigUtils() {}
    UiConfigUtils(const UiConfig& map) : _map(map) {}

    template <typename T>
    T Get(const std::string& configName, const T& defaultValue)
    {
        VariantValue configValueString;
        if (AtUtils::Lookup<std::string, VariantValue>(_map, configName, configValueString))
        {
            T configValue = AtUtils::FromString<T>(configValueString);
            return configValue;
        }
        else
        {
            return defaultValue;
        }
    }

private:
    UiConfig _map;
};