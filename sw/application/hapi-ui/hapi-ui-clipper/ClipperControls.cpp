/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "ClipperControls.h"
#include "UiElements.h"
#include "ClipperUtils.h"
#include "ClipperImplementation.h"

using namespace SwApi;


static constexpr float minRoiWidth = 0.05f;
static constexpr float minRoiHeight = 0.05f;


ClipperControls::ClipperControls(const std::shared_ptr<SwApi::IClipper>& spClipper, bool usePollBehaviour)
:   _spClipper(spClipper)
,   _usePollBehaviour(usePollBehaviour)
{
    _roi._enabled = false;
    _roi._tmo_enabled = false;
    _roi._outside = false;
    _roi._x = 0.0f;
    _roi._y = 0.0f;
    _roi._width = 1.0f;
    _roi._height = 1.0f;
}

std::vector<std::shared_ptr<UiControlContainer>> ClipperControls::AddUiElements()
{
    if (!_spClipper)
        return {};

    auto clippingMode = _spClipper->GetMode();

    auto spContainer = std::make_shared<UiControlContainer>("Clipper", GetSettingsSectionName());

    auto showRoiCB = [this](uint32_t clientID) { ShowClipperRoiDialogUiUpdate({ _spRoiPanel }, clientID); };
    spContainer->AddButtonControl("Edit ROI", showRoiCB);


    _spRoiPanel = std::make_shared<UiControlContainer>("Region of Interest ", "ClipperRoi");
    _spRoiEditorPanel = std::make_shared<UiControlContainer>("RoI Editor", "ClipperRoiEditor");
    _spRoiSettingsPanel = std::make_shared<UiControlContainer>("Region of Interest", "ClipperRoiSettings");
    _spRoiPresetLabelPanel = std::make_shared<UiControlContainer>("Region of Interest", "ClipperRoiPresetLabel");
    _spRoiPresetPanel1 = std::make_shared<UiControlContainer>("Region of Interest", "ClipperRoiPresets");
    _spRoiPresetPanel2 = std::make_shared<UiControlContainer>("Region of Interest", "ClipperRoiPresets");

    _spRoiPanel->Add(_spRoiEditorPanel);
    _spRoiPanel->Add(_spRoiSettingsPanel);
    _spRoiPanel->Add(_spRoiPresetLabelPanel);
    _spRoiPanel->Add(_spRoiPresetPanel1);
    _spRoiPanel->Add(_spRoiPresetPanel2);

    auto roiResetCB = [this] (uint32_t clientID) -> void
    {
        _roi._x = 0.0f;
        _roi._y = 0.0f;
        _roi._width = 1.0f;
        _roi._height = 1.0f;

        UpdateROI();
    };
    _spRoiPanel->AddHeaderButtons( {
        { "RoiReset", roiResetCB, "Reset", "OJL/Images/Reset.png" }
    });

    _spRoiEditorPanel->EnableBorder(false);   
    _spRoiEditorPanel->SetSpacing(0);
    _spRoiEditorPanel->SetTransparent(true);

    _spRoiSettingsPanel->EnableBorder(false);   
    _spRoiSettingsPanel->SetSpacing(0);
    _spRoiSettingsPanel->SetTransparent(true);

    _spRoiPresetLabelPanel->EnableBorder(false);   
    _spRoiPresetLabelPanel->SetSpacing(0);
    _spRoiPresetLabelPanel->SetTransparent(true);

    _spRoiPresetPanel1->EnableBorder(false);   
    _spRoiPresetPanel1->SetSpacing(0);
    _spRoiPresetPanel1->SetTransparent(true);

    _spRoiPresetPanel2->EnableBorder(false);   
    _spRoiPresetPanel2->SetSpacing(0);
    _spRoiPresetPanel2->SetTransparent(true);

    auto roiUpdateCB = [this](uint32_t clientId, RoiSelectorData& value) 
    { 
        // Only copy the intel_vvp_clipper_roi parts
        _roi._x = value._x;
        _roi._y = value._y;
        _roi._width = value._width;
        _roi._height = value._height;
        UpdateROI(true); 
    };
    _spRoiCustomControl = std::make_shared<RoiCustomControl>("OJRoiControl", _roi, roiUpdateCB);
    _spRoiEditorPanel->Add(_spRoiCustomControl);

    switch(clippingMode)
    {
        case ClipperMode::OffsetClipping: {
            auto topCB = [this](uint32_t clientId, float& value) {
                //reduce the height by the change in top offset to preserve the bottom offset 
                if (_roi._height > value - _roi._y + minRoiHeight) _roi._height -= value - _roi._y; 
                else _roi._height = minRoiHeight;
                _roi._y = value; 
                UpdateROI(); 
            };
            _spTopControl = _spRoiSettingsPanel->AddFloatControl("Top Offset", 0.0f, 0.95f, topCB, "roiWinTop", 0.0f);
            auto leftCB = [this](uint32_t clientId, float& value) {
                //reduce the width by the change in left offset to preserve the right offset 
                if (_roi._width > value - _roi._x + minRoiWidth) _roi._width -= value - _roi._x; 
                else _roi._width = minRoiWidth;
                _roi._x = value; 
                UpdateROI();
            };
            _spLeftControl = _spRoiSettingsPanel->AddFloatControl("Left Offset", 0.0f, 0.95f, leftCB, "roiWinLeft", 0.0f);
            auto bottomCB = [this](uint32_t clientId, float& value) {
                if (_roi._y + value > 0.95) _roi._y = 0.95f - value;
                _roi._height = 1.0f - _roi._y - value; 
                UpdateROI();
            };
            _spBottomControl = _spRoiSettingsPanel->AddFloatControl("Bottom Offset", 0.0f, 0.95f, bottomCB, "roiWinBottom", 0.0f);
            auto rightCB = [this](uint32_t clientId, float& value) {
                if (_roi._x + value > 0.95) _roi._x = 0.95f - value;
                _roi._width = 1.0f - _roi._x - value; 
                UpdateROI();
            };
            _spRightControl = _spRoiSettingsPanel->AddFloatControl("Right Offset", 0.0f, 0.95f, rightCB, "roiWinRight", 0.0f);
            break;
        }
        case ClipperMode::RectangleClipping: {
            auto xCB = [this](uint32_t clientId, float& value) {
                if (_roi._width + value > 1.0f) _roi._width = 1.0f - value;
                _roi._x = value; 
                UpdateROI(); 
            };
            _spXControl = _spRoiSettingsPanel->AddFloatControl("X", 0.0f, 0.95f, xCB, "roiWinX", 0.0f);
            auto yCB = [this](uint32_t clientId, float& value) {
                if (_roi._height + value > 1.0f) _roi._height = 1.0f - value;
                _roi._y = value; 
                UpdateROI();
            };
            _spYControl = _spRoiSettingsPanel->AddFloatControl("Y", 0.0f, 0.95f, yCB, "roiWinY", 0.0f);
            auto widthCB = [this](uint32_t clientId, float& value) {
                if (_roi._x + value > 1.0f) _roi._x = 1.0f - value;
                _roi._width = value; 
                UpdateROI();
            };
            _spWidthControl = _spRoiSettingsPanel->AddFloatControl("Width", 0.05f, 1.0f, widthCB, "roiWinWidth", 1.0f);
            auto heightCB = [this](uint32_t clientId, float& value) {
                if (_roi._y + value > 1.0f) _roi._y = 1.0f - value;
                else _roi._height = value; 
                UpdateROI();
            };
            _spHeightControl = _spRoiSettingsPanel->AddFloatControl("Height", 0.05f, 1.0f, heightCB, "roiWinHeight", 1.0f);
            break;
        }
        case ClipperMode::InvalidClipping:
        default: {
            std::cerr << "Invalid clipping mode for ClipperControls!\n";
            break;
        }
    }

    _spRoiPresetLabelPanel->AddLabelControl("Presets");
    int32_t margin = 12;

    auto leftHalfCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.0f; _roi._width = 0.5f; _roi._height = 1.0f; UpdateROI(); };
    _spRoiPresetPanel1->AddButtonControl("Left half", leftHalfCB)->AddAttributes({{ "center", "true" }});
    auto rightHalfCB = [this](uint32_t clientID) { _roi._x = 0.5f; _roi._y = 0.0f; _roi._width = 0.5f; _roi._height = 1.0f; UpdateROI(); };
    _spRoiPresetPanel1->AddButtonControl("Right half", rightHalfCB)->AddAttributes({{ "center", "true" }});
    auto topHalfCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.0f; _roi._width = 1.0f; _roi._height = 0.5f; UpdateROI(); };
    _spRoiPresetPanel1->AddButtonControl("Top half", topHalfCB)->AddAttributes({{ "center", "true" }});
    auto bottomHalfCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.5f; _roi._width = 1.0f; _roi._height = 0.5f; UpdateROI(); };
    _spRoiPresetPanel2->AddButtonControl("Bottom half", bottomHalfCB)->AddAttributes({{ "center", "true" }});
    auto centerCB = [this](uint32_t clientID) { _roi._x = 0.25f; _roi._y = 0.25f; _roi._width = 0.5f; _roi._height = 0.5f; UpdateROI(); };
    _spRoiPresetPanel2->AddButtonControl("Center", centerCB)->AddAttributes({{ "center", "true" }});
    auto fullScreenCB = [this](uint32_t clientID) { _roi._x = 0.0f; _roi._y = 0.0f; _roi._width = 1.0f; _roi._height = 1.0f; UpdateROI(); };
    _spRoiPresetPanel2->AddButtonControl("Full-screen", fullScreenCB)->AddAttributes({{ "center", "true" }});

    _spRoiPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1100 });
    _spRoiPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 780 });

    _spRoiEditorPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiEditorPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spRoiEditorPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 1000 });
    _spRoiEditorPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 585 });
    
    _spRoiSettingsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin }); // Match roi editor start
    _spRoiSettingsPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 585 + margin });
    _spRoiSettingsPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 320 });
    _spRoiSettingsPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 135 });

    _spRoiPresetLabelPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 320 + margin });
    _spRoiPresetLabelPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 570 + margin });
    _spRoiPresetLabelPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 480 });
    _spRoiPresetLabelPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 30 });
    
    _spRoiPresetPanel1->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 320 + margin });
    _spRoiPresetPanel1->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 570 + margin + 30 });
    _spRoiPresetPanel1->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 240 });
    _spRoiPresetPanel1->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    _spRoiPresetPanel2->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 320 + margin + 240 });
    _spRoiPresetPanel2->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, 570 + margin + 30 });
    _spRoiPresetPanel2->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 240 });
    _spRoiPresetPanel2->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 85 });

    _roi._enabled = true;
    _roi._tmo_enabled = true;
    _roi._outside = true;
    _roi._x = 0;
    _roi._y = 0;
    _roi._width = 1;
    _roi._height = 1;
    _ready = true;
    UpdateROI(true);

    // Save these control panels, so they can be added to the dialog box,
    // and they get loaded with settings
    _dialogControls = { _spRoiPanel };

    return {std::move(spContainer)};
}

void ClipperControls::UpdateROI(bool updateUI)
{
    if (_ready)
    {
        // Push the validated _roi to the driver
        switch(_spClipper->GetMode())
        {
            case ClipperMode::RectangleClipping: {
                if (updateUI)
                {
                    _spXControl->UpdateValue(_roi._x);
                    _spYControl->UpdateValue(_roi._y);
                    _spWidthControl->UpdateValue(_roi._width);
                    _spHeightControl->UpdateValue(_roi._height);

                    _spRoiCustomControl->UpdateValue(_roi);
                }
                break;
            }
            case ClipperMode::OffsetClipping: {
                if (updateUI)
                {
                    _spTopControl->UpdateValue(_roi._y);
                    _spLeftControl->UpdateValue(_roi._x);
                    _spRightControl->UpdateValue(1.0f - _roi._x - _roi._width);
                    _spBottomControl->UpdateValue(1.0f - _roi._y - _roi._height);

                    _spRoiCustomControl->UpdateValue(_roi);
                }
                break;
            }
            case ClipperMode::InvalidClipping:
            default: {
                std::cerr << "Invalid clipping mode for ClipperControls!\n";
                break;
            }
        }

        _hasUpdatedPipeline = false;
        if (!_usePollBehaviour)
        {
            PolledUpdateCB();
        }
    }
}

void ClipperControls::PolledUpdateCB()
{
    if (_hasUpdatedPipeline)
    {
        return;
    }

    switch(_spClipper->GetMode())
    {
        case ClipperMode::RectangleClipping: 
        {
            RectangleClipSetting rectRoi;

            rectRoi.leftOffset = (unsigned int)(_roi._x * _spClipper->GetInputWidth()) & (~0x01);
            rectRoi.topOffset = (unsigned int)(_roi._y * _spClipper->GetInputHeight()) & (~0x01);
            rectRoi.clipWidth = (unsigned int)(_roi._width * _spClipper->GetInputWidth()) & (~0x01);
            rectRoi.clipHeight = (unsigned int)(_roi._height * _spClipper->GetInputHeight()) & (~0x01);

            _spClipper->SetClipTo(rectRoi);
            break;
        }
        case ClipperMode::OffsetClipping: 
        {
            OffsetClipSetting offsetRoi;

            offsetRoi.left = (unsigned int)(_roi._x * _spClipper->GetInputWidth()) & (~0x01);
            offsetRoi.top = (unsigned int)(_roi._y * _spClipper->GetInputHeight()) & (~0x01);
            offsetRoi.right = (unsigned int)((1.0f - _roi._x - _roi._width) * _spClipper->GetInputWidth()) & (~0x01);
            offsetRoi.bottom = (unsigned int)((1.0f - _roi._y - _roi._height) * _spClipper->GetInputHeight()) & (~0x01);

            _spClipper->SetClipTo(offsetRoi);
            break;
        }
        case ClipperMode::InvalidClipping:
        default: {
            std::cerr << "Invalid clipping mode for ClipperControls!\n";
            break;
        }
    }

    if (_fpUpdatePipelineWithNewClip)
    {
        _fpUpdatePipelineWithNewClip();
    }

    _hasUpdatedPipeline = true;
}

void ClipperControls::LoadUpdatePipelineCB(std::function<void(void)> fpUpdatePipelineWithNewClip)
{
    _fpUpdatePipelineWithNewClip = std::move(fpUpdatePipelineWithNewClip);
}

///////////////////////////////////////////////////////////////////////////////

ShowClipperRoiDialogUiUpdate::ShowClipperRoiDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                                                   uint32_t clientID)
:   UiUpdate(clientID)
{
    UiUpdateJSON document;
    AtUtils::IJsonObjectPtr spNode = document.GetRoot();
    AtUtils::IJsonObjectPtr spCommandNode = spNode->AddObject("ModalDialog");
    spCommandNode->AddValue("dialog_type", "ISPAWBDialog");
    spCommandNode->AddValue("dialog_title", "Clipper Region of Interest");
    // Hardcoded for now but could be provided as a parameter in the constructor
    spCommandNode->AddValue("width", 1150);

    auto spControlsArray = spCommandNode->AddArray("Controls");

    for (const auto& spControl : controls)
    {
        auto spJsonObject = spControlsArray->AddElement()->AddObject();
        spControl->GetJSON(spJsonObject);
    }

    _jsonString = document.ToString();
    Notify();
}
