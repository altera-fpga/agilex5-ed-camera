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
#include "IClipper.h"
#include "IspCommon.h"

/// Encapsulates UI functionality for the Clipper IP.
class ClipperControls : public IpUiControls
{
    using RoiCustomControl = UiCustomControlWithValue<RoiSelectorData>;

public:
    /// Creates an instance of the class that'll manage the given Clipper instance.
    ClipperControls(const std::shared_ptr<SwApi::IClipper>& spClipper, bool usePollBehaviour = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "Clipper"; };

    void PolledUpdateCB();
    void LoadTogglePollCB(std::function<void(bool)> fpTogglePoll);
    void LoadUpdatePipelineCB(std::function<void(void)> fpUpdatePipelineWithNewClip);

private:
    std::shared_ptr<SwApi::IClipper> _spClipper;

    void UpdateROI(bool updateUI = true);

    std::shared_ptr<UiControlContainer>     _spRoiPanel;
	std::shared_ptr<UiControlContainer>     _spRoiEditorPanel;
	std::shared_ptr<UiControlContainer>     _spRoiSettingsPanel;
    std::shared_ptr<UiControlContainer>     _spRoiPresetLabelPanel;
    std::shared_ptr<UiControlContainer>     _spRoiPresetPanel1;
    std::shared_ptr<UiControlContainer>     _spRoiPresetPanel2;

    std::shared_ptr<UiControlItemFloat>     _spTopControl;
    std::shared_ptr<UiControlItemFloat>     _spLeftControl;
    std::shared_ptr<UiControlItemFloat>     _spBottomControl;
    std::shared_ptr<UiControlItemFloat>     _spRightControl;
    std::shared_ptr<UiControlItemFloat>     _spXControl;
    std::shared_ptr<UiControlItemFloat>     _spYControl;
    std::shared_ptr<UiControlItemFloat>     _spWidthControl;
    std::shared_ptr<UiControlItemFloat>     _spHeightControl;
    std::shared_ptr<RoiCustomControl>       _spRoiCustomControl;

    RoiSelectorData _roi;
    bool _ready = false;

    bool _usePollBehaviour = false;
    bool _hasUpdatedPipeline = true;

    std::function<void(bool)> _fpTogglePoll;
    std::function<void(void)> _fpUpdatePipelineWithNewClip;
};

class ShowClipperRoiDialogUiUpdate : public UiUpdate
{
public:
    ShowClipperRoiDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                             uint32_t clientID = allClients);
};