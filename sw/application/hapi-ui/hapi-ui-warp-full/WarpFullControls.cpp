/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpFullControls.h"
#include "CommonApplicationBase.h"
#include "WarpConfiguratorTypes.h"
#include "AtUtils.h"
#include "WarpAdapter.h"
#include <cmath>

#ifdef __linux__
#include <unistd.h>
#endif

static const char* meshFileTitle = "warp-mesh.owf";
static const uint32_t panelWidth = 370;
static constexpr double systemClock = 300000000.0;

WarpFullControls::WarpFullControls(const std::string& name,
                                   std::shared_ptr<SwApi::WarpAdapter> spWarpAdapter,
                                   bool useDialog,
                                   CapturePictureCB captureCB,
                                   EnableWarpCB enableWarpCB,
                                   WarpControlMode defaultMode)
:   _name(name)
,   _spWarpAdapter{spWarpAdapter}
,   _captureCB{captureCB}
,   _enableWarpCB{enableWarpCB}
,   _useDialog{useDialog}
,   _defaultMode{defaultMode}
,   _meshEditorLeft{panelWidth + 13}
,   _meshEditorWidth(useDialog ? 1300 : 1510)
,   _meshEditorHeight(useDialog ? 850 : 972)
,   _enableDebugControls{false}
{
}

void WarpFullControls::UpdateWarpAdapter()
{
    // Don't update the warp adapter during initialisation
    if (_pauseUpdateAdapter)
        return;

    // Need to push the PreRadial value for Fixed and Corners since the warp adapter
    // only stores one value for this but the two control panels keep
    // their own independent values
    if (_mode == WarpControlMode::Fixed)
    {
        {
            const auto k1_new = _spPreRadialSliderFixed->GetValue<float>();
            auto configurator = _spWarpAdapter->GetConfiguratorSafe();
            
            float x = 0.0f;
            float y = 0.0f;
            float k1 = 0.0f;
            float k2 = 0.0f;

            configurator->GetPreRadial(x, y, k1, k2);
            configurator->SetPreRadial(x, y, k1_new, k2);
        }

        _spWarpAdapter->UpdateFixed();
    }
    else if (_mode == WarpControlMode::Corners)
    {
        {
            const auto k1_new = _spPreRadialSliderCorners->GetValue<float>();
            auto configurator = _spWarpAdapter->GetConfiguratorSafe();

            float x = 0.0f;
            float y = 0.0f;
            float k1 = 0.0f;
            float k2 = 0.0f;

            configurator->GetPreRadial(x, y, k1, k2);
            configurator->SetPreRadial(x, y, k1_new, k2);
        }

        _spWarpAdapter->UpdateCorners();
    }
    else if (_mode == WarpControlMode::Arbitrary)
    {
        _spWarpAdapter->UpdateArbitrary();
    }
    else if (_mode == WarpControlMode::Fisheye)
    {
        _spWarpAdapter->UpdateFisheye();
    }
}

// Fetch the latest arbitrary warp points and corner points,
// and update the controls _spArbitraryControl and _spCornersControl
// with these values
void WarpFullControls::UpdateControlPoints()
{
    const auto configurator = _spWarpAdapter->GetConfiguratorSafe();

    const uint32_t num_knots = configurator->GetArbitraryKnotsNum();
    WarpPointsValue arbitraryPoints(num_knots, num_knots);

    for(uint32_t i = 0; i < (num_knots * num_knots); ++i)
    {
        arbitraryPoints._points[i] = configurator->GetArbitraryKnotNorm(i);
    }

    _spArbitraryControl->UpdateValue(arbitraryPoints);

    WarpPointsValue cornerPoints(2, 2);
    
    cornerPoints._points[0] = configurator->GetCornerNorm(intel_vvp_warp::ETopLeft);
    cornerPoints._points[1] = configurator->GetCornerNorm(intel_vvp_warp::ETopRight);
    cornerPoints._points[2] = configurator->GetCornerNorm(intel_vvp_warp::EBottomLeft);
    cornerPoints._points[3] = configurator->GetCornerNorm(intel_vvp_warp::EBottomRight);
    _spCornersControl->UpdateValue(cornerPoints);
}

// Slider values 0, 1, 2, 3, 4 are mapped to 2, 3, 5, 9 and 17
int32_t WarpFullControls::KnotsValueToDisplay(int32_t value)
{
    int32_t display_value = 2;
    for (int32_t i = 0; i < value; i++)
        display_value += (display_value - 1);

    return display_value;
}

int32_t WarpFullControls::KnotsDisplayToValue(int32_t display_value)
{
    int32_t display = 2;
    for (int32_t i = 0; i < 5; i++)
    {
        if (display == display_value)
            return i;
        display += (display - 1);
        if (display > display_value)
            return i;
    }

    return 0;
}

void WarpFullControls::ConvertToArbitraryMesh(bool fromCorners)
{
    auto configurator = _spWarpAdapter->GetConfiguratorSafe();
    uint32_t numKnots = configurator->ConvertToArbitraryMesh(fromCorners);

    WarpPointsValue warpPoints(numKnots, numKnots);

    for (uint32_t i = 0; i < (numKnots * numKnots); ++i)
    {
        warpPoints._points[i] = configurator->GetArbitraryKnotNorm(i);
    }

    int32_t numKnotsValue = KnotsDisplayToValue((int32_t)numKnots);
    _spNumArbitraryKnotsSlider->UpdateValue(numKnotsValue, false);
    _spArbitraryKnotsParameter->UpdateValue(numKnots);

    _spArbitraryControl->UpdateValue(warpPoints);

    // Auto switch to arbitrary controls
    _spWarpMode->UpdateValue(2, true);
};

std::vector<std::shared_ptr<UiControlContainer>> WarpFullControls::AddUiElements()
{
    if (!_spWarpAdapter)
        return {};

    _spMeshEditorPanel = std::make_shared<UiControlContainer>("Mesh Editor", GetSettingsSectionName() + "_MeshEditor");
    _spFixedControlsPanel = std::make_shared<UiControlContainer>("Fixed Controls", GetSettingsSectionName() + "_Fixed");
    _spCornersControlPanel = std::make_shared<UiControlContainer>("Corner Controls", GetSettingsSectionName() + "_Corner");
    _spArbitraryControlsPanel = std::make_shared<UiControlContainer>("Arbitrary Controls", GetSettingsSectionName() + "_Arbitrary");
    _spFisheyeControlsPanel = std::make_shared<UiControlContainer>("Fisheye Controls", GetSettingsSectionName() + "_Fisheye");
    auto commonPanel = std::make_shared<UiControlContainer>("Miscellaneous Controls", GetSettingsSectionName());
    _spWarpMiscPanel = std::make_shared<UiControlContainer>("", GetSettingsSectionName() + "_Misc");
    _spModeContainerPanel = std::make_shared<UiControlContainer>("", GetSettingsSectionName() + "_Mode");
    _spModeContainerPanel->SetSpacing(0);
    _spModeContainerPanel->SetTransparent(true);
    _spModeContainerPanel->EnableBorder(false);   

    _spWarningsPanel = std::make_shared<UiControlContainer>("", GetSettingsSectionName() + "_WarningsPanel");
    _spWarningsPanel->SetSpacing(0);
    _spWarningsPanel->SetTransparent(true);
    _spWarningsPanel->EnableBorder(false);

    auto customFillXmlCB = [this](AtUtils::IJsonObjectPtr& spJsonObject)
    {
        auto [outputWidth, outputHeight] = _spWarpAdapter->GetOutputResolution();
        UpdateControlPoints();

        spJsonObject->AddValue("output_width", outputWidth);
        spJsonObject->AddValue("output_height", outputHeight);
        spJsonObject->AddValue("mode", (uint32_t)_mode);
        spJsonObject->AddValue("mesh_warning", AtUtils::ToString(!_validTransformation));  // Client code expects string value here
    };

    _spWarpCustomControl = std::make_shared<UiCustomControl>("OJWarpFullControl", 
                                                             customFillXmlCB);
    uint32_t warpCustomControlID = _spWarpCustomControl->GetID();			
    _spMeshEditorPanel->Add(_spWarpCustomControl);

    // Dialog mode doesn't support switching to fullscreen
    if (!_useDialog)
        _spMeshEditorPanel->AddHeaderButtons({{ "", nullptr, "Maximize", "OJL/Images/Maximise.png", "OnMaximize" }});    

    _spMeshEditorPanel->SetSpacing(12);
    _spMeshEditorPanel->Add(_spWarpMiscPanel);
    _spMeshEditorPanel->Add(_spModeContainerPanel);
    _spMeshEditorPanel->Add(_spWarningsPanel);

    _modeChangeStart = std::make_shared<UiControlItemHiddenUInt32>(0, warpCustomControlID, "ModeChangeStartCB");
    _modeChangeComplete = std::make_shared<UiControlItemHiddenUInt32>(0, warpCustomControlID, "ModeChangeCompleteCB");
    
    // Common controls
    auto mainBypassCB = [this](uint32_t clientID, bool& value) { Bypass(value, _spMainBypassControl); };
    _spMainBypassControl = commonPanel->AddBoolControl("Bypass", mainBypassCB, "Bypass", _spWarpAdapter->GetBypass());
    _spMainBypassControl->SetClientCallbackName(warpCustomControlID, "BypassWarpCB");

    if(_spWarpAdapter->IsSingleBounce())
        _spMainBypassControl->Enable(false);

    // Low frame rate fall back. Only available in double bounce configuration
    if(false == _spWarpAdapter->IsSingleBounce())
    {
        auto enableLowFrameRateCB = [this](uint32_t clientID, bool& value) 
        { 
            _spWarpAdapter->SetLowFramerateFallback(value);
        };
        auto lfrButton = commonPanel->AddBoolControl("Low frame rate mode", enableLowFrameRateCB, "LFR", true);    										 
        lfrButton->SetToolTip("Enable/disable the low frame rate mode");
    }

    commonPanel->AddSeparator();
    _totalLatencyLabel = commonPanel->AddLabelControl("Minimal total latency", "-");

    static constexpr bool canSetLatency = false;
    if (canSetLatency)
    {
        auto setLatencyCB = [this](uint32_t clientID, float& value) { _spWarpAdapter->SetLatency((uint32_t)((value / 1000.0) * systemClock)); };
        commonPanel->AddFloatControl("Set latency", 0.0f, 50.0f, setLatencyCB, "Latency", 0.0f)->SetValuePreAndPostfix("", "ms");
    }

    // Print warp debug registers
    if(_enableDebugControls)
    {
        commonPanel->AddSeparator();

        auto debugCB = [this](uint32_t clientID) 
        {
            _spWarpAdapter->PrintDebugRegisters();
        };

        commonPanel->AddButtonControl("Debug", debugCB);
    }

    // Fixed Controls /////////////////////////////////////////////////////////
    const bool mipmap_enabled = _spWarpAdapter->IsMipmapEnabled();

    auto rotationCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetRotate(value); UpdateWarpAdapter();
    };
    _spRotationSlider = _spFixedControlsPanel->AddSliderControl("Rotation", -180.0f, 180.0f, rotationCB, "Rotation", _spWarpAdapter->GetConfiguratorSafe()->GetRotate());
    _spRotationSlider->SetClientCallbackName(warpCustomControlID, "RotationCB");

    auto zoomCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetZoom(value); UpdateWarpAdapter();
    };

    // Due to Agilex 5 bandwidth limitation default range is limited
    // to avoid output video loss
    // Full range is available in "power user" mode
    auto zoom_range = std::make_pair(0.7f, 2.0f);
    
    if(_enableDebugControls)
        zoom_range =  mipmap_enabled ? std::make_pair(0.03125f, 2.0f) : std::make_pair(0.5f, 2.0f);    

    _spZoomSlider = _spFixedControlsPanel->AddSliderControl("Zoom", zoom_range.first, zoom_range.second, zoomCB, "Zoom", _spWarpAdapter->GetConfiguratorSafe()->GetZoom());
    _spZoomSlider->SetClientCallbackName(warpCustomControlID, "ZoomCB");

    auto horizontalOffsetCB = [this](uint32_t clientID, int32_t& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetHOffset(value); UpdateWarpAdapter();
    };
    _spHorizontalOffsetSlider = _spFixedControlsPanel->AddSliderControl("Horizontal offset", -240 * 8, 240 * 8, horizontalOffsetCB, "HOffset", _spWarpAdapter->GetConfiguratorSafe()->GetHOffset());
    _spHorizontalOffsetSlider->SetClientCallbackName(warpCustomControlID, "HorizontalOffsetCB");

    auto verticalOffsetCB = [this](uint32_t clientID, int32_t& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetVOffset(value); UpdateWarpAdapter();
    };
    _spVerticalOffsetSlider = _spFixedControlsPanel->AddSliderControl("Vertical offset", -240 * 4, 240 * 4, verticalOffsetCB, "VOffset", _spWarpAdapter->GetConfiguratorSafe()->GetVOffset());
    _spVerticalOffsetSlider->SetClientCallbackName(warpCustomControlID, "VerticalOffsetCB");

    auto horizontalMirrorCB = [this](uint32_t clientID, bool& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetHMirror(value); UpdateWarpAdapter();
    };
    _spHorizontalMirrorCheckbox = _spFixedControlsPanel->AddBoolControl("Horizontal mirror", horizontalMirrorCB, "HMirror", _spWarpAdapter->GetConfiguratorSafe()->GetHMirror());
    _spHorizontalMirrorCheckbox->SetClientCallbackName(warpCustomControlID, "HorizontalMirrorCB");

    auto verticalMirrorCB = [this](uint32_t clientID, bool& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetVMirror(value); UpdateWarpAdapter();
    };
    _spVerticalMirrorCheckbox = _spFixedControlsPanel->AddBoolControl("Vertical mirror", verticalMirrorCB, "VMirror", _spWarpAdapter->GetConfiguratorSafe()->GetVMirror());
    _spVerticalMirrorCheckbox->SetClientCallbackName(warpCustomControlID, "VerticalMirrorCB");

    auto horizontalKeystoneCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetHKeystone(value); UpdateWarpAdapter();
    };
    const auto hkey_range = mipmap_enabled ? std::make_pair(-65.0f, 65.0f) : std::make_pair(-30.0f, 30.0f);
    _spHorizontalKeystoneSlider = _spFixedControlsPanel->AddSliderControl("Horizontal keystone", hkey_range.first, hkey_range.second, horizontalKeystoneCB, "HKeystone", _spWarpAdapter->GetConfiguratorSafe()->GetHKeystone());
    _spHorizontalKeystoneSlider->SetClientCallbackName(warpCustomControlID, "HorizontalKeystoneCB");

    auto verticalKeystoneCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetVKeystone(value); UpdateWarpAdapter();
    };
    const auto vkey_range = mipmap_enabled ? std::make_pair(-65.0f, 65.0f) : std::make_pair(-30.0f, 30.0f);
    _spVerticalKeystoneSlider = _spFixedControlsPanel->AddSliderControl("Vertical keystone", vkey_range.first, vkey_range.second, verticalKeystoneCB, "VKeystone", _spWarpAdapter->GetConfiguratorSafe()->GetVKeystone());
    _spVerticalKeystoneSlider->SetClientCallbackName(warpCustomControlID, "VerticalKeystoneCB");        

    auto preRadialCB = [this](uint32_t clientID, float& value) {
        auto configurator = _spWarpAdapter->GetConfiguratorSafe();

        float x = 0.0f;
        float y = 0.0f;
        float k1 = 0.0f;
        float k2 = 0.0f;

        configurator->GetPreRadial(x, y, k1, k2);
        configurator->SetPreRadial(x, y, value, k2);

        UpdateWarpAdapter();
    };

    _spPreRadialSliderFixed = _spFixedControlsPanel->AddSliderControl("Radial distortion", -0.45f, 3.00f, preRadialCB, "PreRadial", 0.0f);
    _spPreRadialSliderFixed->SetClientCallbackName(warpCustomControlID, "FixedRadialCB");

    auto resetFixedCB = [this](uint32_t clientID) 
    {
        // Clear the transformation
        auto configurator = _spWarpAdapter->GetConfiguratorSafe();
        configurator->SetRotate(0.0f);
        configurator->SetZoom(1.0f);
        configurator->SetHOffset(0);
        configurator->SetVOffset(0);
        configurator->SetHMirror(false);
        configurator->SetVMirror(false);
        configurator->SetHKeystone(0.0f);
        configurator->SetVKeystone(0.0f);

        float x = 0.0f;
        float y = 0.0f;
        float k1 = 0.0f;
        float k2 = 0.0f;  
        
        configurator->GetPreRadial(x, y, k1, k2);
        configurator->SetPreRadial(x, y, 0.0f, k2);
        
        // Reset the UI controls
        _spRotationSlider->UpdateValue(0.0f);
        _spZoomSlider->UpdateValue(1.0f);
        _spHorizontalOffsetSlider->UpdateValue(0);
        _spVerticalOffsetSlider->UpdateValue(0);
        _spHorizontalMirrorCheckbox->UpdateValue(false);
        _spVerticalMirrorCheckbox->UpdateValue(false);
        _spHorizontalKeystoneSlider->UpdateValue(0.0f);
        _spVerticalKeystoneSlider->UpdateValue(0.0f);
        _spPreRadialSliderFixed->UpdateValue(0.0f);

        UpdateWarpAdapter();
    };

    auto convertFixedToArbitraryCB = [this](uint32_t clientID) { ConvertToArbitraryMesh(false); };

    _spFixedControlsPanel->AddHeaderButtons( {
        { "FixedToArbitraryButton", convertFixedToArbitraryCB, "Convert to arbitrary mesh", "OJL/Images/ConvertMesh.png" },
        { "FixedControlsReset", resetFixedCB, "Reset", "OJL/Images/Reset.png" }
    });

    // Corner Controls ////////////////////////////////////////////////////////
    _spPreRadialSliderCorners = _spCornersControlPanel->AddSliderControl("Radial distortion", -0.45f, 3.00f, preRadialCB, "PreRadial", 0.0f);
    _spPreRadialSliderCorners->SetClientCallbackName(warpCustomControlID, "CornerRadialCB");

    auto cornerCB = [this](uint32_t clientID, uint32_t numKnots, uint32_t cornerID, float x, float y) {
        if(cornerID < intel_vvp_warp::ETotalCorners)
        {
            auto configurator = _spWarpAdapter->GetConfiguratorSafe();
            configurator->SetCornerNorm(static_cast<intel_vvp_warp::ECornerId>(cornerID), x, y);
            UpdateWarpAdapter();
        }
    };

    auto configurator = _spWarpAdapter->GetConfiguratorSafe();

    WarpPointsValue cornerPoints(2, 2);
    cornerPoints._points[0] = configurator->GetCornerNorm(intel_vvp_warp::ETopLeft);
    cornerPoints._points[1] = configurator->GetCornerNorm(intel_vvp_warp::ETopRight);
    cornerPoints._points[2] = configurator->GetCornerNorm(intel_vvp_warp::EBottomLeft);
    cornerPoints._points[3] = configurator->GetCornerNorm(intel_vvp_warp::EBottomRight);
    _spCornersControl = std::make_shared<UiControlItemWarpCorners>("Corner position", cornerPoints, cornerCB, warpCustomControlID);
    _spCornersControl->SetClientCallbackName(warpCustomControlID, "CornerCB");

    _spMeshEditorPanel->Add(_spCornersControl, "CornerPoints", cornerPoints.ToString(true));

    auto resetCornersCB = [this](uint32_t clientID) 
    {
        // Reset the Corner UI controls
        _spPreRadialSliderCorners->UpdateValue(0.0f);

        auto configurator = _spWarpAdapter->GetConfiguratorSafe();

        {
            float x = 0.0f;
            float y = 0.0f;
            float k1 = 0.0f;
            float k2 = 0.0f;  
            
            configurator->GetPreRadial(x, y, k1, k2);
            configurator->SetPreRadial(x, y, 0.0f, k2);
        }

        configurator->ResetCorners();
        auto v = configurator->GetCornerNorm(intel_vvp_warp::ETopLeft);
        _spCornersControl->UpdateValue(0, v.first, v.second);
        v = configurator->GetCornerNorm(intel_vvp_warp::ETopRight);
        _spCornersControl->UpdateValue(1, v.first, v.second);
        v = configurator->GetCornerNorm(intel_vvp_warp::EBottomLeft);
        _spCornersControl->UpdateValue(2, v.first, v.second);
        v = configurator->GetCornerNorm(intel_vvp_warp::EBottomRight);
        _spCornersControl->UpdateValue(3, v.first, v.second);

        UpdateWarpAdapter();
    };

    auto convertCornersToArbitraryCB = [this](uint32_t clientID) { ConvertToArbitraryMesh(true); };

    _spCornersControlPanel->AddHeaderButtons( {
        { "CornersToArbitraryButton", convertCornersToArbitraryCB, "Convert to arbitrary mesh", "OJL/Images/ConvertMesh.png" },
        { "CornerControlsReset", resetCornersCB, "Reset", "OJL/Images/Reset.png" }
    });        

    // Arbitrary Controls /////////////////////////////////////////////////////
    auto numArbitraryKnotsCB = [this](uint32_t clientID, int32_t& value) 
    {
        int32_t display_value = KnotsValueToDisplay(value);

        auto configurator = _spWarpAdapter->GetConfiguratorSafe();

        configurator->SetArbitraryKnotsNum(display_value, true);
        UpdateWarpAdapter();

        const uint32_t num_knots = configurator->GetArbitraryKnotsNum();
        WarpPointsValue warpPoints(num_knots, num_knots);

        for(uint32_t i = 0; i < (num_knots * num_knots); ++i)
        {
            warpPoints._points[i] = configurator->GetArbitraryKnotNorm(i);
        }

        _spArbitraryKnotsParameter->UpdateValue(num_knots);
        _spArbitraryControl->UpdateValue(warpPoints);
    };

    const uint32_t num_knots = configurator->GetArbitraryKnotsNum();
    int32_t numKnotsValue = KnotsDisplayToValue((int32_t)num_knots);
    _spNumArbitraryKnotsSlider = _spArbitraryControlsPanel->AddSliderControl("Control points", numKnotsValue, 0, 4, numArbitraryKnotsCB);

    // JS Conversion functions to run on the client
    std::string valueToDisplayFn = "WarpPointsValueToDisplay";
    std::string displayToValueFn = "WarpPointsDisplayToValue";
    _spNumArbitraryKnotsSlider->SetValueToDisplayConverters(warpCustomControlID, valueToDisplayFn, displayToValueFn);

    auto arbitraryCB = [this](uint32_t clientID, uint32_t totalNumKnots, uint32_t pointID, float x, float y) 
    {
        auto configurator = _spWarpAdapter->GetConfiguratorSafe();

        uint32_t numKnots = (uint32_t)(std::sqrt((double)totalNumKnots) + 0.01);
        if (numKnots != configurator->GetArbitraryKnotsNum())
        {
            configurator->SetArbitraryKnotsNum(numKnots);

            int32_t numKnotsValue = KnotsDisplayToValue((int32_t)numKnots);
            _spNumArbitraryKnotsSlider->UpdateValue(numKnotsValue);
        }

        configurator->SetArbitraryKnotNorm(pointID, x, y);
        UpdateWarpAdapter();
    };

    WarpPointsValue arbitraryPoints(num_knots, num_knots);

    for(uint32_t i = 0; i < (num_knots * num_knots); ++i)
    {
        arbitraryPoints._points[i] = configurator->GetArbitraryKnotNorm(i);
    }

    _spArbitraryControl = std::make_shared<UiControlItemWarpArbitrary>("Point position", arbitraryPoints, arbitraryCB, warpCustomControlID);
    _spArbitraryControl->SetClientCallbackName(warpCustomControlID, "ArbitraryKnotsPosCB");
    _spMeshEditorPanel->Add(_spArbitraryControl, "ArbitraryPoints", arbitraryPoints.ToString(true));        
    _spArbitraryControlsPanel->AddSeparator();

    auto exportMeshCB = [this](uint32_t clientID) 
    { 
        std::filesystem::path meshFilename = std::filesystem::current_path();
        meshFilename /= meshFileTitle;

        if (_spWarpAdapter->ExportMesh(meshFilename, false))
        {
            ExportFileUiUpdate exportFile(clientID, meshFilename);
        }
    };
    _spArbitraryControlsPanel->AddButtonControl("Export mesh", exportMeshCB);

    auto importMeshCB = [this](uint32_t clientID) 
    { 
        std::filesystem::path meshFilename = std::filesystem::current_path();
        meshFilename /= meshFileTitle;

        auto importCompleteAction = [this, clientID](std::filesystem::path tempFilePath, 
                                                     std::filesystem::path importDestinationPath) 
        { 
            WarpPointsValue warpPoints;

            std::string tempFileExtension = tempFilePath.extension();
            AtUtils::MakeLower(tempFileExtension);

            std::string importFileExtension = importDestinationPath.extension();
            AtUtils::MakeLower(importFileExtension);

            std::filesystem::path importMeshPath;

            if (tempFileExtension == importFileExtension)
            {
                std::error_code ec;
                std::filesystem::copy(tempFilePath, importDestinationPath,
                                      std::filesystem::copy_options::overwrite_existing, ec);
                std::filesystem::remove(tempFilePath, ec);
                importMeshPath = importDestinationPath;
#ifdef __linux__
                ::sync();
#endif                    
            }
            else
            {
                // If importing a .json for example, don't copy to a .owf
                importMeshPath = tempFilePath;
            }

            if (_spWarpAdapter->ImportMesh(importMeshPath, warpPoints._points, warpPoints._nRows, warpPoints._nColumns))
            {
                int32_t numKnotsValue = KnotsDisplayToValue((int32_t)warpPoints._nRows);
                _spNumArbitraryKnotsSlider->UpdateValue(numKnotsValue, false);
                _spArbitraryKnotsParameter->UpdateValue(warpPoints._nRows);
                _spArbitraryControl->UpdateValue(warpPoints);
            }
            else
            {
                UiMessage("Failed to load mesh file");
            }
        };
        ImportFileUiUpdate importFile(clientID, std::move(meshFilename), "Warp Mesh", importCompleteAction);
    };

    _spArbitraryControlsPanel->AddButtonControl("Import mesh", importMeshCB);

    auto resetArbitraryCB = [this](uint32_t clientID) 
    {
        auto configurator = _spWarpAdapter->GetConfiguratorSafe();
        // Reset the Arbitrary controls
        uint32_t num_knots = configurator->GetArbitraryKnotsNum();
        configurator->SetArbitraryKnotsNum(num_knots);

        WarpPointsValue warpPoints(num_knots, num_knots);

        for(uint32_t i = 0; i < (num_knots * num_knots); ++i)
        {
            warpPoints._points[i] = configurator->GetArbitraryKnotNorm(i);
        }

        int32_t numKnotsValue = KnotsDisplayToValue((int32_t)num_knots);
        _spNumArbitraryKnotsSlider->UpdateValue(numKnotsValue, false);
        _spArbitraryKnotsParameter->UpdateValue(num_knots);
        _spArbitraryControl->UpdateValue(warpPoints);
        UpdateWarpAdapter();       
    };    

    _spArbitraryControlsPanel->AddHeaderButtons( {
        { "ArbitraryControlsReset", resetArbitraryCB, "Reset", "OJL/Images/Reset.png" }
    });

    // Fisheye Controls /////////////////////////////////////////////////////

    auto fisheyeLensFovCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFisheyeLensFov(value); UpdateWarpAdapter();
    };

    _spFisheyeLensFovSlider = _spFisheyeControlsPanel->AddSliderControl("Lens FOV", 120.0f, 220.0f, fisheyeLensFovCB, "FisheyeLensFov", 180.0f);
    _spFisheyeLensFovSlider->SetClientCallbackName(warpCustomControlID, "FisheyeLensFovCB");

    auto fisheyeRadiusCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFisheyeRadius(value); UpdateWarpAdapter();
    };
    _spFisheyeRadiusSlider = _spFisheyeControlsPanel->AddSliderControl("Fisheye Radius", 0.8f, 1.4f, fisheyeRadiusCB, "FisheyeRadius", 1.0f);
    _spFisheyeRadiusSlider->SetClientCallbackName(warpCustomControlID, "FtpRadiusCB");

    // Fisheye mapping selector
    _spFisheyeControlsPanel->AddSeparator();

    std::vector<UiEnumOption> fisheyeModeOptions = {
        { "Panorama", (uint32_t)0 },
        { "Equirectangular", (uint32_t)1 },
        { "None", (uint32_t)2 }
    };

    auto fisheyeModeCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void {
        auto fisheyeMode = (uint32_t)selected._userItemData;

        auto configurator = _spWarpAdapter->GetConfiguratorSafe();
        
        if(fisheyeMode == 0)
        {
            configurator->SetEquiRectEnable(false);
            configurator->SetFishToPanoEnable(true);
        }
        else if(fisheyeMode == 1)
        {
            configurator->SetFishToPanoEnable(false);
            configurator->SetEquiRectEnable(true);
        }
        else if(fisheyeMode == 2)
        {
            configurator->SetFishToPanoEnable(false);
            configurator->SetEquiRectEnable(false);
        }

        UpdateWarpAdapter();
    };

    _spFisheyeModeDropdown = _spFisheyeControlsPanel->AddEnumControl("Mapping", fisheyeModeOptions, fisheyeModeCB, "FisheyeMode", 2);
    _spFisheyeModeDropdown->SetClientCallbackName(warpCustomControlID, "FisheyeModeCB");

    ////// Fisheye to panorama mapping controls
    _spFisheyeControlsPanel->AddSeparator();

     _spFisheyeControlsPanel->AddLabelControl("Panorama settings");

    auto fisheyeTabLatitudeMinCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoLatitudeMin(value); UpdateWarpAdapter();
    };

    _spFisheyePanoLatitudeMinSlider = _spFisheyeControlsPanel->AddSliderControl("Latitude min", 0.0f, 80.0f, fisheyeTabLatitudeMinCB, "FtpLattitudeMin", 0.0f);
    _spFisheyePanoLatitudeMinSlider->SetClientCallbackName(warpCustomControlID, "FtpLatitudeMinCB");

    auto fisheyeTabLatitudeMaxCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoLatitudeMax(value); UpdateWarpAdapter();
    };
    _spFisheyePanoLatitudeMaxSlider = _spFisheyeControlsPanel->AddSliderControl("Latitude max", 10.0f, 90.0f, fisheyeTabLatitudeMaxCB, "FtpLattitudeMax", 90.0f);
    _spFisheyePanoLatitudeMaxSlider->SetClientCallbackName(warpCustomControlID, "FtpLatitudeMaxCB");

    auto fisheyeTabLongitudeCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoLongitude(value); UpdateWarpAdapter();
    };
    _spFisheyePanoLongitudeSlider = _spFisheyeControlsPanel->AddSliderControl("Longitude min", 0.0f, 360.0f, fisheyeTabLongitudeCB, "FtpLongitudeMin", 0.0f);
    _spFisheyePanoLongitudeSlider->SetClientCallbackName(warpCustomControlID, "FtpLongitudeCB");

    auto fisheyeTabHorizontalFovCB = [this](uint32_t clientID, float& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoFovH(value); UpdateWarpAdapter();
    };
    _spFisheyePanoHorizontalFovSlider = _spFisheyeControlsPanel->AddSliderControl("Horizontal FOV", 30.0f, 360.0f, fisheyeTabHorizontalFovCB, "FtpLongitudeMax", 90.0f);
    _spFisheyePanoHorizontalFovSlider->SetClientCallbackName(warpCustomControlID, "FtpHorizontalFovCB");

    auto fisheyeTabInsideOutCB = [this](uint32_t clientID, bool& value) {
        _spWarpAdapter->GetConfiguratorSafe()->SetFtpInsideOut(value); UpdateWarpAdapter();
    };

    auto spFisheyeTabInsideOut = _spFisheyeControlsPanel->AddBoolControl("InsideOut", fisheyeTabInsideOutCB, "InsideOut", true);
    spFisheyeTabInsideOut->SetClientCallbackName(warpCustomControlID, "FtpInsideOutCB");    

    auto resetFtpCB = [this](uint32_t clientID) 
    {
        // Clear the transformation
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoLatitudeMin(15.0f);
        _spFisheyePanoLatitudeMinSlider->UpdateValue(15.0f);
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoLatitudeMax(56.0f);
        _spFisheyePanoLatitudeMaxSlider->UpdateValue(56.0f);
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoLongitude(0.0f);
        _spFisheyePanoLongitudeSlider->UpdateValue(0.0f);
        _spWarpAdapter->GetConfiguratorSafe()->SetFishToPanoFovH(90.0f);
        _spFisheyePanoHorizontalFovSlider->UpdateValue(90.0f);
        _spWarpAdapter->GetConfiguratorSafe()->SetFisheyeRadius(1.2f);
        _spFisheyeRadiusSlider->UpdateValue(1.2f);
        UpdateWarpAdapter();
    };    

    _spFisheyeControlsPanel->AddHeaderButtons( {
        { "FtpControlsReset", resetFtpCB, "Reset", "OJL/Images/Reset.png" }
    });

    ////// Equirectangular projection for fisheye lens
    _spFisheyeControlsPanel->AddSeparator();

    _spFisheyeControlsPanel->AddLabelControl("Equirectangular settings");
    
    auto equiRectFovVCB = [this](uint32_t clientID, float& value) {
        auto configurator = _spWarpAdapter->GetConfiguratorSafe();
        configurator->SetEquiRectFovV(value);
        UpdateWarpAdapter();
    };
    _spFisheyeEquirectVFovSlider = _spFisheyeControlsPanel->AddSliderControl("Vertical FOV", 60.0f, 186.0f, equiRectFovVCB, "EquirectVFov", 90.0f);
    _spFisheyeEquirectVFovSlider->SetClientCallbackName(warpCustomControlID, "EquirectVFovCB");
    
    auto equiRectFovHCB = [this](uint32_t clientID, float& value) {
        auto configurator = _spWarpAdapter->GetConfiguratorSafe();
        configurator->SetEquiRectFovH(value);
        UpdateWarpAdapter();
    };
    _spFisheyeEquirectHFovSlider = _spFisheyeControlsPanel->AddSliderControl("Horizontal FOV", 60.0f, 186.0f, equiRectFovHCB, "EquirectHFov", 90.0f);
    _spFisheyeEquirectHFovSlider->SetClientCallbackName(warpCustomControlID, "EquirectHFovCB");

    // Hidden Controls //////////////////////////////////////////////////////
    
    auto res = _spWarpAdapter->GetOutputResolution();
    _spOutputResolution = std::make_shared<UiControlItemHiddenResolution>(res, warpCustomControlID, "ResolutionCB");
    _spArbitraryKnotsParameter = std::make_shared<UiControlItemHiddenUInt32>(num_knots, warpCustomControlID, "ArbitraryKnotsNumCB");

    _spMeshEditorPanel->Add(_spOutputResolution);
    _spMeshEditorPanel->Add(_spArbitraryKnotsParameter);
    _spMeshEditorPanel->Add(_modeChangeStart);
    _spMeshEditorPanel->Add(_modeChangeComplete);

    _spWarpMiscPanel->EnableBorder(false);   
    _spWarpMiscPanel->SetSpacing(0);
    _spWarpMiscPanel->SetTransparent(true);

    // Switching between warp modes
    std::vector<UiEnumOption> warpModeOptions = 
    {
        {"Fixed", WarpControlMode::Fixed, "OJL/Images/WarpFixed.png"},
        {"Corners", WarpControlMode::Corners, "OJL/Images/WarpCorners.png"},
        {"Arbitrary", WarpControlMode::Arbitrary, "OJL/Images/WarpArbitrary.png"},
        {"Fisheye", WarpControlMode::Fisheye, "OJL/Images/WarpFisheye.png"}
    };
    
    // Allow UpdateWarpAdapter to proceed now
    _pauseUpdateAdapter = false;

    auto warpModeCB = [this](uint32_t clientId, const UiEnumOption &selectedOption, uint32_t) 
    {
        uint32_t mode = selectedOption._userItemData;

        if (mode < WarpControlMode::Max)
        {
            WarpControlMode oldMode = _mode;
            _mode = static_cast<WarpControlMode>(mode);
            if (_mode != oldMode)
            {
                _modeChangeStart->UpdateValue(mode);

                UpdateWarpAdapter();
                UpdateModePanels();

                if (_mode == WarpControlMode::Fixed)
                    _spFixedControlsPanel->PushValueToUI(UiUpdate::allClients);
                else if (_mode == WarpControlMode::Corners)
                {
                    _spCornersControl->PushValueToUI(UiUpdate::allClients);
                    _spCornersControlPanel->PushValueToUI(UiUpdate::allClients);
                }
                else if (_mode == WarpControlMode::Arbitrary)
                {
                    _spArbitraryControl->PushValueToUI(UiUpdate::allClients);
                    _spArbitraryControlsPanel->PushValueToUI(UiUpdate::allClients);
                }
                else if (_mode == WarpControlMode::Fisheye)
                {
                    _spFisheyeControlsPanel->PushValueToUI(UiUpdate::allClients);
                }                

                _modeChangeComplete->UpdateValue(mode);
            }
        }
    };
    
    _spWarpMode = _spModeContainerPanel->AddEnumRadioControl(warpModeOptions, warpModeCB, "Mode", (uint32_t)_defaultMode);
    _mode = (WarpControlMode)_spWarpMode->GetSelectedIndex();

    auto meshViewCB = [this](uint32_t clientID, float& value) { };
    auto spMeshScaler = _spWarpMiscPanel->AddSliderControl("Scale view", 0.25f, 1.0f, meshViewCB, "meshScale", 1.0f);
    spMeshScaler->SetClientCallbackName(warpCustomControlID, "MeshScaleCB");

    auto showAlignmentCB = [this](uint32_t clientID, bool& value) { };
    auto spShowAlignmentGuide = _spWarpMiscPanel->AddBoolControl("Show alignment guide", showAlignmentCB, "ShowAlignment", true);
    spShowAlignmentGuide->SetClientCallbackName(warpCustomControlID, "ShowAlignmentGuideCB");

    int32_t margin = 12;
    int32_t offset = _useDialog ? margin : 0;

    commonPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset });
    commonPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset });
    commonPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
    commonPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

    _spMeshEditorPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, _meshEditorLeft + offset});
    _spMeshEditorPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset});
    _spMeshEditorPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, _meshEditorWidth });
    _spMeshEditorPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, _meshEditorHeight });

    _spFixedControlsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset });
    _spFixedControlsPanel->SetTopAnchor({ ANCHOR_TYPE::SIBLING_FAR, commonPanel->GetID(), margin});
    _spFixedControlsPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
    _spFixedControlsPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

    _spCornersControlPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset });
    _spCornersControlPanel->SetTopAnchor({ ANCHOR_TYPE::SIBLING_FAR, commonPanel->GetID(), margin});
    _spCornersControlPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
    _spCornersControlPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

    _spArbitraryControlsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset });
    _spArbitraryControlsPanel->SetTopAnchor({ ANCHOR_TYPE::SIBLING_FAR, commonPanel->GetID(), margin});
    _spArbitraryControlsPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
    _spArbitraryControlsPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

    _spFisheyeControlsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, offset });
    _spFisheyeControlsPanel->SetTopAnchor({ ANCHOR_TYPE::SIBLING_FAR, commonPanel->GetID(), margin});
    _spFisheyeControlsPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, panelWidth });
    _spFisheyeControlsPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 0 });

    _spWarpMiscPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, margin });
    _spWarpMiscPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_FAR, -80 });
    _spWarpMiscPanel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 400 });
    _spWarpMiscPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 75 });

    _spModeContainerPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 480 });
    _spModeContainerPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_FAR, -80 });
    _spModeContainerPanel->SetRightAnchor({ ANCHOR_TYPE::PARENT_FAR, -480 });
    _spModeContainerPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 48 });

    _spWarningsPanel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_FAR, -80 - margin });
    _spWarningsPanel->SetTopAnchor({ ANCHOR_TYPE::PARENT_FAR, -68 });
    _spWarningsPanel->SetRightAnchor({ ANCHOR_TYPE::PARENT_FAR, -margin });
    _spWarningsPanel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 48 });

    _lfrWarningLabel = _spWarningsPanel->AddLabelControl("LFR");
    _lfrWarningLabel->OverrideStyle({{"color", "#f04040"},
                                     {"border", "solid 1px #f04040"},
                                     {"backgroundColor", "#e9d2d2"},
                                     {"borderRadius", "8px"}});

    // Add a command to poll the mesh validity and the latency
    AtUtils::AutoRepeatCommandQueueCB pollMeshCB = [=, this](uint32_t&) 
    {
        auto adapterMode = _spWarpAdapter->GetMode();
        switch(adapterMode)
        {
        case SwApi::WarpAdapter::WarpAdapterMode::Fixed:
            if (_mode != WarpControlMode::Fixed)
            {
                // switch to fixed controls
                _spWarpMode->UpdateValue(0, true);
            }
            break;
        case SwApi::WarpAdapter::WarpAdapterMode::Corners:
            if (_mode != WarpControlMode::Corners)
            {
                // switch to corners controls
                _spWarpMode->UpdateValue(1, true);
            }
            break;
        case SwApi::WarpAdapter::WarpAdapterMode::Arbitrary:
            if (_mode != WarpControlMode::Arbitrary)
            {
                // switch to arbtrary controls
                _spWarpMode->UpdateValue(2, true);
            }
            break;
        case SwApi::WarpAdapter::WarpAdapterMode::Fisheye:
            if (_mode != WarpControlMode::Fisheye)
            {
                // switch to fisheye controls
                _spWarpMode->UpdateValue(3, true);
            }
            break;
        }

        bool validTransformation = _spWarpAdapter->IsValidTransformation();
        if (validTransformation != _validTransformation)
        {
            _validTransformation = validTransformation;
            CallJavaScriptFunction(warpCustomControlID, "SetMeshWarning", !_validTransformation);
        }

        auto latencyParameters = _spWarpAdapter->GetLatencyParams();
        double totalLatencySecs = (double)latencyParameters._total_latency / systemClock;
        double totalLatencymSec = totalLatencySecs * 1000.0;

        if (!_warpBypass)
            _totalLatencyLabel->UpdateValue(AtUtils::ToString(totalLatencymSec, "%0.2f ms"));
        else
            _totalLatencyLabel->UpdateValue("-");
            
        return true;
    };

    CommonApplicationBase::Get()->AddCommand(pollMeshCB, 500); // 2 Hz

    // Add a command to poll the low-frame-rate state
    AtUtils::AutoRepeatCommandQueueCB pollLFR = [=, this](uint32_t&) 
    {
        bool lfrEnabled = _spWarpAdapter->GetLowFramerateFallback();
        bool showLfrWarning = lfrEnabled && _spWarpAdapter->GetLowFrameRateDetected();
        if (showLfrWarning != _showLfrWarning)
        {
            _showLfrWarning = showLfrWarning;
            _lfrWarningLabel->Show(_showLfrWarning);
        }

        return true;
    };

    CommonApplicationBase::Get()->AddCommand(pollLFR, 67); // About 15Hz
    
    // Warp panel needs to be first as others reference
    // the OJWarpFullControl (use for forwarding callbacks) in the JS
    if (!_useDialog)
    {
        return { _spMeshEditorPanel, commonPanel, _spFixedControlsPanel,
                 _spCornersControlPanel, _spArbitraryControlsPanel, _spFisheyeControlsPanel };
    }

    // Save these control panels, so they can be added to the dialog box,
    // and they get loaded with settings
    _dialogControls = { _spMeshEditorPanel, commonPanel,
                        _spFixedControlsPanel, _spCornersControlPanel, 
                        _spArbitraryControlsPanel, _spFisheyeControlsPanel };

    auto spWarpContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());
    auto windowBypassCB = [this](uint32_t clientID, bool& value) { Bypass(value, _spWindowBypassControl); };
    _spWindowBypassControl = spWarpContainer->AddBoolControl("Bypass", windowBypassCB, "Bypass", _spWarpAdapter->GetBypass());

    if(_spWarpAdapter && _spWarpAdapter->IsSingleBounce())
        _spWindowBypassControl->Enable(false);

    auto expandCollapseCB = [this](uint32_t clientID) 
    {
        auto [output_width, output_height] = _spWarpAdapter->GetOutputResolution();
        UpdateControlPoints();
        ShowWarpDialogUiUpdate(_dialogControls, output_width, output_height, clientID);
    };
    spWarpContainer->AddButtonControl("Show controls", expandCollapseCB);

    if(_enableDebugControls)
    {
        spWarpContainer->AddSeparator();
        
        auto debugCB = [this](uint32_t clientID) 
        {
            _spWarpAdapter->PrintDebugRegisters();
        };

        spWarpContainer->AddButtonControl("Debug", debugCB);

        if (_captureCB && !_spWarpAdapter->IsSingleBounce())
        {
            spWarpContainer->AddPictureCaptureControl("Capture Picture", ModalDialogUiUpdate::DialogType::PICTURE, _captureCB);
        }        
    }



    return {spWarpContainer};
}

void WarpFullControls::Bypass(bool state, std::shared_ptr<UiControlItemBoolean> originButton)
{
    if (_enableWarpCB)
        _enableWarpCB(!state);
    else if (_spWarpAdapter)
        _spWarpAdapter->SetBypass(state);

    // Update matching control
    if (originButton)
    {
        if (originButton == _spWindowBypassControl)
            _spMainBypassControl->UpdateValue(state);
        else if (originButton == _spMainBypassControl)
            _spWindowBypassControl->UpdateValue(state);
    }

    _warpBypass = state;
}

void WarpFullControls::UpdateModePanels()
{
    bool updateNow = _controlsLoaded;

    bool showFixed = false;
    bool showCorners = false;
    bool showArbitrary = false;
    bool showFisheye = false;
    
    if (_mode == WarpControlMode::Fixed)
        showFixed = true;
    else if (_mode == WarpControlMode::Corners)
        showCorners = true;
    else if (_mode == WarpControlMode::Arbitrary)
        showArbitrary = true;
    else if (_mode == WarpControlMode::Fisheye)
        showFisheye = true;

    _spFixedControlsPanel->Show(showFixed, updateNow);
    _spCornersControlPanel->Show(showCorners, updateNow);
    _spArbitraryControlsPanel->Show(showArbitrary, updateNow);
    _spFisheyeControlsPanel->Show(showFisheye, updateNow);
}

void WarpFullControls::UpdateOutputResolution(uint32_t width, uint32_t height)
{
    _spOutputResolution->UpdateValue(std::make_pair(width, height));
    UpdateControlPoints();
}

void WarpFullControls::EnableDebugControls(const bool enable)
{
    _enableDebugControls = enable;
}

///////////////////////////////////////////////////////////////////////////////

WarpEasyControls::WarpEasyControls(std::shared_ptr<SwApi::WarpAdapter> spWarpAdapter,
                                   CapturePictureCB captureCB,
                                   EasyWarpRotationCB easyWarpRotationCB)
:   _spWarpAdapter{spWarpAdapter}
,   _captureCB{captureCB}
,   _easyWarpRotationCB{easyWarpRotationCB}
{
}

std::vector<std::shared_ptr<UiControlContainer>> WarpEasyControls::AddUiElements()
{
    auto spEasyWarpContainer = std::make_shared<UiControlContainer>("Easy Warp", GetSettingsSectionName());

    auto horizontalMirrorCB = [this](uint32_t clientID, bool& value) { _spWarpAdapter->SetEasyWarpMirror(value); };
    _spHorizontalMirrorCheckbox = spEasyWarpContainer->AddBoolControl("Horizontal mirror", horizontalMirrorCB, "Mirror", false);

    auto rotationCB = [this](uint32_t clientId, const UiEnumOption& selectedOption, uint32_t) 
    {
        SwApi::WarpAdapter::EasyWarpRotation rotation = (SwApi::WarpAdapter::EasyWarpRotation)selectedOption._userItemData;

        if (_easyWarpRotationCB)
            _easyWarpRotationCB(rotation);
    };
    std::vector<UiEnumOption> rotationOptions;
    rotationOptions.push_back({ "None", (uint32_t)SwApi::WarpAdapter::EasyWarpRotation::None});
    rotationOptions.push_back({ "90 degrees clockwise", (uint32_t)SwApi::WarpAdapter::EasyWarpRotation::R90});
    rotationOptions.push_back({ "180 degrees", (uint32_t)SwApi::WarpAdapter::EasyWarpRotation::R180});
    rotationOptions.push_back({ "90 degrees anticlockwise", (uint32_t)SwApi::WarpAdapter::EasyWarpRotation::R270});

    _spRotationDropdown = spEasyWarpContainer->AddEnumControl("Rotation", 
                                                              rotationOptions, 
                                                              rotationCB,
                                                              "Rotation",
                                                              0);

    if (_captureCB)
    {
        spEasyWarpContainer->AddPictureCaptureControl("Capture Picture", ModalDialogUiUpdate::DialogType::PICTURE, _captureCB);
    }

    return { spEasyWarpContainer };
}


void WarpEasyControls::EnableControls(bool state)
{
    if (_spHorizontalMirrorCheckbox)
        _spHorizontalMirrorCheckbox->Enable(state);
    if (_spRotationDropdown)
        _spRotationDropdown->Enable(state);
}

void WarpEasyControls::SetRotationControl(SwApi::WarpAdapter::EasyWarpRotation rotation)
{
    if (_spRotationDropdown)
        _spRotationDropdown->UpdateValue((uint32_t)rotation);
}

///////////////////////////////////////////////////////////////////////////////

ShowWarpDialogUiUpdate::ShowWarpDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls, 
                                               uint32_t outputWidth,
                                               uint32_t outputHeight, 
                                               uint32_t clientID)
:   UiUpdate(clientID)
{
    UiUpdateJSON document;
    AtUtils::IJsonObjectPtr spNode = document.GetRoot();
    AtUtils::IJsonObjectPtr spCommandNode = spNode->AddObject("ModalDialog");
    spCommandNode->AddValue("dialog_type", "WarpControls");
    spCommandNode->AddValue("output_width", outputWidth);
    spCommandNode->AddValue("output_height", outputHeight);

    auto spControlsArray = spCommandNode->AddArray("Controls");

    for (const auto& spControl : controls)
    {
        auto spJsonObject = spControlsArray->AddElement()->AddObject();
        spControl->GetJSON(spJsonObject);
    }

    _jsonString = document.ToString();
    Notify();
}

