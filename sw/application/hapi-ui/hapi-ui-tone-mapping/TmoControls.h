/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "IpUiControls.h"
#include "ITmo.h"
#include "IspCommon.h"

class TmoControls : public IpUiControls
{
    using RoiCustomControl = UiCustomControlWithValue<RoiSelectorData>;

public:
    TmoControls(const std::shared_ptr<SwApi::ITmo>& spTmo);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { return "ToneMapping"; }

private:
    void UpdateROI(bool updateUI = true);

    std::shared_ptr<SwApi::ITmo> _spTmo;
	std::shared_ptr<UiControlContainer>     _spRoiEditorPanel;
	std::shared_ptr<UiControlContainer>     _spRoiSettingsPanel;
    std::shared_ptr<UiControlItemBoolean>   _spMainEnableControl;
    std::shared_ptr<UiControlItemBoolean>   _spWindowEnableControl;    
    std::shared_ptr<UiControlItemFloat>     _spXControl;
    std::shared_ptr<UiControlItemFloat>     _spYControl;
    std::shared_ptr<UiControlItemFloat>     _spWidthControl;
    std::shared_ptr<UiControlItemFloat>     _spHeightControl;
    std::shared_ptr<RoiCustomControl>       _spRoiCustomControl;

    std::shared_ptr<UiControlItemSlider>    _spMainLevelControl;
    std::shared_ptr<UiControlItemSlider>    _spWindowLevelControl;
    std::shared_ptr<UiControlItemSlider>    _spMainThresholdControl;
    std::shared_ptr<UiControlItemSlider>    _spWindowThresholdControl;
    RoiSelectorData _roi = {};
    bool _ready = false;
    int32_t _roiEditorLeft = 0;
    int32_t _roiEditorWidth = 0;
    int32_t _roiEditorHeight = 0;
};

class ShowTmoRoiDialogUiUpdate : public UiUpdate
{
public:
    ShowTmoRoiDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                             uint32_t clientID = allClients);
};
