/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <deque>
#include "IpUiControls.h"


class UiLoggerControl : public UiControlItem
{
public:
    UiLoggerControl();

    AtUtils::JsonValueVariant UpdateValue(const std::string& value);

    bool SaveToFile(const std::string& filename);

protected:
    virtual void FillJson(AtUtils::IJsonObjectPtr &spJsonObject) override;

private:
    std::deque<std::string> _data;
};


class LoggerControl: public IpUiControls
{
public:
    LoggerControl();
    virtual ~LoggerControl() = default;
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
    std::string GetSettingsSectionName() override { return ""; }

    void LogMessage(const std::string& msg);

private:
    std::shared_ptr<UiLoggerControl>	_uiControl;    
};
