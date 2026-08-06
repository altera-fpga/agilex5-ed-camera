/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "IpUiControls.h"
#include "CommonUiUpdate.h"

void IpUiControls::SetManualPositionCB(ManualPositionCB manualPositionCB)
{
    _manualPositionCB = std::move(manualPositionCB);
}

// Return true if a manual position callback was supplied
bool IpUiControls::DoManualPosition(std::vector<std::shared_ptr<UiControlContainer>>& containers)
{
    if (_manualPositionCB)
    {
        _manualPositionCB(containers);
        return true;
    }
    else
    {
        return false;
    }
}

void IpUiControls::SetControlsLoaded()
{
    _controlsLoaded = true;
}

bool IpUiControls::ControlsAreLoaded()
{
    return _controlsLoaded;
}

void IpUiControls::CallJavaScriptFunction(uint32_t controlID, std::string functionName, bool parameter)
{
    std::string stringParameter = AtUtils::ToString(parameter);
    UiUpdate::CallJavaScriptFunction(controlID, std::move(functionName), std::move(stringParameter));
}

void IpUiControls::CallJavaScriptFunction(uint32_t controlID, std::string functionName, int32_t parameter)
{
    std::string stringParameter = std::to_string(parameter);
    UiUpdate::CallJavaScriptFunction(controlID, std::move(functionName), std::move(stringParameter));
}

void IpUiControls::CallJavaScriptFunction(uint32_t controlID, std::string functionName, AtUtils::IJsonPtr parameter)
{
    std::string stringParameter = parameter->ToString();
    UiUpdate::CallJavaScriptFunction(controlID, std::move(functionName), std::move(stringParameter));
}

void IpUiControls::CallJavaScriptFunction(uint32_t controlID, std::string functionName, std::string parameter)
{
    UiUpdate::CallJavaScriptFunction(controlID, std::move(functionName), std::move(parameter));
}

void IpUiControls::AddColumnBreak(std::vector<std::shared_ptr<UiControlContainer>>& panels)
{
    panels.push_back(std::make_shared<ColumnBreak>());
}
