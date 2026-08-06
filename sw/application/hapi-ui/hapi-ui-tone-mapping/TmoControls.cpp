/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "TmoControls.h"

static const uint32_t panelWidth = 370;
static constexpr bool    factorySettingBypass = true;
static constexpr int32_t factorySettingThreshold = 5000;
static constexpr int32_t factorySettingLevel = 60;

TmoControls::TmoControls(const std::shared_ptr<SwApi::ITmo>& spTmo)
    : _spTmo(spTmo),
      _roiEditorLeft{panelWidth + 13},
      _roiEditorWidth(1300),
      _roiEditorHeight(785)
{
}

std::vector<std::shared_ptr<UiControlContainer>> TmoControls::AddUiElements()
{
    if (!_spTmo)
        return {};

    auto spContainer = std::make_shared<UiControlContainer>("Tone Mapping", GetSettingsSectionName());

    auto toggleBypassCB = [this](uint32_t clientID, bool& value)
        {
            _roi._tmo_enabled = !value; // Inverted because it's a bypass value
            _spTmo->SetBypass(value);
            if (_spWindowEnableControl)
                _spWindowEnableControl->UpdateValue(value);

            // Switch the ROI editor colors
            if (_spRoiCustomControl)
                _spRoiCustomControl->UpdateValue(_roi);
        };
    _spMainEnableControl = spContainer->AddBoolControl("Bypass", toggleBypassCB, "BypassTMO", factorySettingBypass);

    auto mainThresholdCB = [this](uint32_t clientID, int32_t& value)
        {
            _spTmo->SetThreshold(value);
            if (_spWindowThresholdControl)
                _spWindowThresholdControl->UpdateValue(value);
        };
    _spMainThresholdControl = spContainer->AddSliderControl("Threshold", 0, 9000, mainThresholdCB, "TmoThreshold", factorySettingThreshold);

    auto mainLevelCB = [this](uint32_t clientID, int32_t& value)
        {
            _spTmo->SetLevel(value);
            if (_spWindowLevelControl)
                _spWindowLevelControl->UpdateValue(value);
        };
    _spMainLevelControl = spContainer->AddSliderControl("Level", 0, 100, mainLevelCB, "TmoLevel", factorySettingLevel);

    auto showRoiCB = [this](uint32_t clientID) { ShowTmoRoiDialogUiUpdate({ _spRoiSettingsPanel, _spRoiEditorPanel }, clientID); };
    spContainer->AddButtonControl("Edit ROI", showRoiCB);

	_spRoiSettingsPanel = std::make_shared<UiControlContainer>("Controls", "TmoRoi");

    /*
    auto windowEnableCB = [this](uint32_t clientID, bool& value)
        {
            _roi._tmo_enabled = !value; // Inverted because it's a bypass value
            _spTmo->SetBypass(value);
            if (_spMainEnableControl)
                _spMainEnableControl->UpdateValue(value);

            // Switch the ROI editor colors
            if (_spRoiCustomControl)
                _spRoiCustomControl->UpdateValue(_roi);
        };
    */
    _spWindowEnableControl = _spRoiSettingsPanel->AddBoolControl("Bypass", toggleBypassCB, "BypassTMO", factorySettingBypass);

    auto windowThresholdCB = [this](uint32_t clientID, int32_t& value)
        {
            _spTmo->SetThreshold(value);
            if (_spMainThresholdControl)
                _spMainThresholdControl->UpdateValue(value);
        };
    _spWindowThresholdControl = _spRoiSettingsPanel->AddSliderControl("Threshold", 0, 9000, windowThresholdCB, "TmoThreshold", factorySettingThreshold);

    auto windowLevelCB = [this](uint32_t clientID, int32_t& value)
        {
            _spTmo->SetLevel(value);
            if (_spMainLevelControl)
                _spMainLevelControl->UpdateValue(value);
        };
    _spWindowLevelControl = _spRoiSettingsPanel->AddSliderControl("Level", 0, 100, windowLevelCB, "TmoLevel", factorySettingLevel);

    _spRoiSettingsPanel->AddSeparator();

    auto enableRoiCB = [this](uint32_t clientId, bool& value)
        {
            _roi._enabled = value;
            _spTmo->SetEnableRoi(value);

            // Show/hide the ROI editor
            if (_spRoiCustomControl)
                _spRoiCustomControl->UpdateValue(_roi);
        };
    _spRoiSettingsPanel->AddBoolControl("Enable ROI", enableRoiCB, "roiEnable", false);
	auto roiOutsideCB = [this](uint32_t clientId, bool& value)
        {
            _roi._outside = value;
            _spTmo->SetRoiOutside(value);

            // Switch the ROI editor colors
            if (_spRoiCustomControl)
                _spRoiCustomControl->UpdateValue(_roi);
        };
    _spRoiSettingsPanel->AddBoolControl("ROI Outside", roiOutsideCB, "roiOutside", false);

    auto xCB = [this](uint32_t clientId, float& value) { _roi._x = value; UpdateROI(); };
    _spXControl = _spRoiSettingsPanel->AddFloatControl("X", 0.0f, 0.875, xCB, "roiX", 0.0625f);
    auto yCB = [this](uint32_t clientId, float& value) { _roi._y = value; UpdateROI(); };
    _spYControl = _spRoiSettingsPanel->AddFloatControl("Y", 0.0f, 0.7778f, yCB, "roiY", 0.1111f );
    auto widthCB = [this](uint32_t clientId, float& value) { _roi._width = value; UpdateROI(); };
    _spWidthControl = _spRoiSettingsPanel->AddFloatControl("Width", 0.0625f, 1.0f, widthCB, "roiWidth", 0.75f);
    auto heightCB = [this](uint32_t clientId, float& value) { _roi._height = value; UpdateROI(); };
    _spHeightControl = _spRoiSettingsPanel->AddFloatControl("Height", 0.1111f, 1.0f, heightCB, "roiHeight", 0.5555f);

    _spRoiSettingsPanel->AddSeparator();

    _spRoiSettingsPanel->AddLabelControl("Presets");
    int32_t margin = 12;

    auto leftHalfCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.0f; _roi._width = 0.5f; _roi._height = 1.0f; UpdateROI(); };
    _spRoiSettingsPanel->AddButtonControl("Left half", leftHalfCB)->AddAttributes({{ "center", "true" }});
    auto rightHalfCB = [this](uint32_t clientID) { _roi._x = 0.5f; _roi._y = 0.0f; _roi._width = 0.5f; _roi._height = 1.0f; UpdateROI(); };
    _spRoiSettingsPanel->AddButtonControl("Right half", rightHalfCB)->AddAttributes({{ "center", "true" }});
    auto topHalfCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.0f; _roi._width = 1.0f; _roi._height = 0.5f; UpdateROI(); };
    _spRoiSettingsPanel->AddButtonControl("Top half", topHalfCB)->AddAttributes({{ "center", "true" }});
    auto bottomHalfCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.5f; _roi._width = 1.0f; _roi._height = 0.5f; UpdateROI(); };
    _spRoiSettingsPanel->AddButtonControl("Bottom half", bottomHalfCB)->AddAttributes({{ "center", "true" }});
    auto centerCB = [this](uint32_t clientID) { _roi._x = 0.25f; _roi._y = 0.25f; _roi._width = 0.5f; _roi._height = 0.5f; UpdateROI(); };
    _spRoiSettingsPanel->AddButtonControl("Center", centerCB)->AddAttributes({{ "center", "true" }});

    _spRoiSettingsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiSettingsPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiSettingsPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
    _spRoiSettingsPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

	_spRoiEditorPanel = std::make_shared<UiControlContainer>("Region Editor", "TmoRoiEditor");
    _spRoiEditorPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, _roiEditorLeft + margin });
    _spRoiEditorPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiEditorPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, _roiEditorWidth });
    _spRoiEditorPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, _roiEditorHeight });

    auto roiUpdateCB = [this](uint32_t clientId, RoiSelectorData& value)
        {
            // Only copy the intel_vvp_tmo_roi parts
            _roi._x = value._x;
            _roi._y = value._y;
            _roi._width = value._width;
            _roi._height = value._height;
            UpdateROI(true);
        };
    _spRoiCustomControl = std::make_shared<RoiCustomControl>("OJRoiControl", _roi, roiUpdateCB);
    _spRoiEditorPanel->Add(_spRoiCustomControl);

    _ready = true;
    UpdateROI(false);

    // Save these control panels, so they can be added to the dialog box,
    // and they get loaded with settings
    _dialogControls = { _spRoiSettingsPanel, _spRoiEditorPanel };

    return {std::move(spContainer)};
}

void TmoControls::UpdateROI(bool updateUI)
{
    if (_ready)
    {
        // Push the validated _roi to the driver

        _roi._x = std::clamp(_roi._x, 0.0f, 1.0f);
        _roi._y = std::clamp(_roi._y, 0.0f, 1.0f);
        _roi._width = std::clamp(_roi._width, 0.0f, 1.0f - _roi._x);
        _roi._height = std::clamp(_roi._height, 0.0f, 1.0f - _roi._y);

        if (updateUI)
        {
            _spXControl->UpdateValue(_roi._x);
            _spYControl->UpdateValue(_roi._y);
            _spWidthControl->UpdateValue(_roi._width);
            _spHeightControl->UpdateValue(_roi._height);
            _spRoiCustomControl->UpdateValue(_roi);
        }

        intel_vvp_tmo_roi vvp_roi = {
            ._x = _roi._x,
            ._y = _roi._y,
            ._width = _roi._width,
            ._height = _roi._height
        };

        _spTmo->SetRegionOfInterest(vvp_roi);
    }
}

///////////////////////////////////////////////////////////////////////////////

ShowTmoRoiDialogUiUpdate::ShowTmoRoiDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                                                   uint32_t clientID)
:   UiUpdate(clientID)
{
    UiUpdateJSON document;
    AtUtils::IJsonObjectPtr spNode = document.GetRoot();
    AtUtils::IJsonObjectPtr spCommandNode = spNode->AddObject("ModalDialog");
    spCommandNode->AddValue("dialog_type", "TmoRoiDialog");
    spCommandNode->AddValue("dialog_title", "Tone Mapping Region of Interest");

    auto spControlsArray = spCommandNode->AddArray("Controls");

    for (const auto& spControl : controls)
    {
        auto spJsonObject = spControlsArray->AddElement()->AddObject();
        spControl->GetJSON(spJsonObject);
    }

    _jsonString = document.ToString();
    Notify();
}
