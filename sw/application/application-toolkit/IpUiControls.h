/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "UiElements.h"

using ManualPositionCB = std::function<void(std::vector<std::shared_ptr<UiControlContainer>>& containers)>;

class IpUiControls
{
public:
    virtual std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() = 0;
	virtual std::string GetSettingsSectionName() = 0;
    void SetManualPositionCB(ManualPositionCB manualPositionCB);
    bool DoManualPosition(std::vector<std::shared_ptr<UiControlContainer>>& containers);
    void SetControlsLoaded();
    bool ControlsAreLoaded();
    void CallJavaScriptFunction(uint32_t objectID, std::string functionName, bool parameter);
    void CallJavaScriptFunction(uint32_t objectID, std::string functionName, int32_t parameter);
    void CallJavaScriptFunction(uint32_t objectID, std::string functionName, std::string parameter);
    void CallJavaScriptFunction(uint32_t objectID, std::string functionName, AtUtils::IJsonPtr parameter);
    std::vector<std::shared_ptr<UiControlContainer>> GetDialogControls() { return _dialogControls; }

    template <class T>
    std::shared_ptr<ISettingValue<T>> CreateSettingValue(const std::string& settingName, const T& defaultValue)
    {
        return std::make_shared<SettingValue<T>>(GetSettingsSectionName().c_str(), settingName.c_str(), defaultValue);
    }

    void AddColumnBreak(std::vector<std::shared_ptr<UiControlContainer>>& panels);

protected:
    bool _controlsLoaded = false;
    std::vector<std::shared_ptr<UiControlContainer>> _dialogControls;
    ManualPositionCB _manualPositionCB;
};

class ColumnBreakPanel : public IpUiControls
{
public:
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override
    {
        return { std::make_shared<ColumnBreak>() };
    }

	std::string GetSettingsSectionName() override { return ""; }
};
