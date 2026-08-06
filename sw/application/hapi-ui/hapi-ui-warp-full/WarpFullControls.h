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
#include "WarpAdapter.h"
#include <functional>
#include <thread>

class UiControlGroup;

using EnableWarpCB = std::function<void(bool)>;

class WarpFullControls : public IpUiControls
{
public:
    enum WarpControlMode {
        Uninitialized = -1,
        Fixed = 0,
        Corners,
        Arbitrary,
        Fisheye,
        Max
    };

    WarpFullControls(const std::string& name,
                     std::shared_ptr<SwApi::WarpAdapter> spWarpAdapter,
					 bool useDialog,
                     CapturePictureCB captureCB = nullptr,
                     EnableWarpCB enableWarpCB = nullptr,
                     WarpControlMode defaultMode = WarpControlMode::Fixed);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

    void UpdateOutputResolution(uint32_t width, uint32_t height);

    void EnableDebugControls(const bool enable);

private:
	void UpdateControlPoints();
	void UpdateWarpAdapter();
	void UpdateModePanels();
    int32_t KnotsValueToDisplay(int32_t value);
    int32_t KnotsDisplayToValue(int32_t display_value);
    void ConvertToArbitraryMesh(bool fromCorners);
    void Bypass(bool state, std::shared_ptr<UiControlItemBoolean> originButton);

    const std::string _name;

    std::shared_ptr<SwApi::WarpAdapter>    _spWarpAdapter;
    std::shared_ptr<UiControlItemBoolean>   _spMainBypassControl;
    std::shared_ptr<UiControlItemBoolean>   _spWindowBypassControl;

    // Fixed controls
    std::shared_ptr<UiControlItemSlider>    _spRotationSlider;
    std::shared_ptr<UiControlItemSlider>    _spZoomSlider;
    std::shared_ptr<UiControlItemSlider>    _spHorizontalOffsetSlider;
    std::shared_ptr<UiControlItemSlider>    _spVerticalOffsetSlider;
    std::shared_ptr<UiControlItemSlider>    _spHorizontalKeystoneSlider;
    std::shared_ptr<UiControlItemSlider>    _spVerticalKeystoneSlider;
    std::shared_ptr<UiControlItemBoolean>   _spHorizontalMirrorCheckbox;
    std::shared_ptr<UiControlItemBoolean>   _spVerticalMirrorCheckbox;
    std::shared_ptr<UiControlItemSlider>    _spPreRadialSliderFixed;
    
    // Corner tab
    std::shared_ptr<UiControlItemSlider>    _spPreRadialSliderCorners;
    std::shared_ptr<UiControlItemWarpCorners> _spCornersControl;

    // Arbitrary tab
    std::shared_ptr<UiControlItemSlider>    _spNumArbitraryKnotsSlider;
    std::shared_ptr<UiControlItemWarpArbitrary> _spArbitraryControl;

    // Fisheye tab
    // Fisheye lens parameter controls
    std::shared_ptr<UiControlItemSlider>    _spFisheyeLensFovSlider;
    std::shared_ptr<UiControlItemSlider>    _spFisheyeRadiusSlider;

    // Fisheye mapping selector
    std::shared_ptr<UiControlItemEnum>      _spFisheyeModeDropdown;

    //// Fisheye to panorama mapping
    std::shared_ptr<UiControlItemSlider>    _spFisheyePanoLatitudeMinSlider;
    std::shared_ptr<UiControlItemSlider>    _spFisheyePanoLatitudeMaxSlider;
    std::shared_ptr<UiControlItemSlider>    _spFisheyePanoLongitudeSlider;
    std::shared_ptr<UiControlItemSlider>    _spFisheyePanoHorizontalFovSlider;

    //// Equirectangular projection
    std::shared_ptr<UiControlItemSlider>    _spFisheyeEquirectVFovSlider;
    std::shared_ptr<UiControlItemSlider>    _spFisheyeEquirectHFovSlider;    

    // Warp mode selector
    std::shared_ptr<UiControlItemEnumRadio> _spWarpMode;

    // Warp output resolution
    std::shared_ptr<UiControlItemHiddenResolution>	_spOutputResolution;
	std::shared_ptr<UiControlItemHiddenUInt32>		_spArbitraryKnotsParameter;
	std::shared_ptr<UiControlItemHiddenUInt32>		_modeChangeStart;
	std::shared_ptr<UiControlItemHiddenUInt32>		_modeChangeComplete;

	std::shared_ptr<UiCustomControl>                _spWarpCustomControl;
    std::shared_ptr<UiControlItemLabel>             _lfrWarningLabel;
    std::shared_ptr<UiControlItemLabel>             _totalLatencyLabel;

    const WarpControlMode _defaultMode;
    WarpControlMode _mode = WarpControlMode::Uninitialized;
    CapturePictureCB _captureCB;
    EnableWarpCB _enableWarpCB;
	bool _useDialog = false;
    bool _pauseUpdateAdapter = true;
    bool _validTransformation = true;
    bool _showLfrWarning = true;
    bool _warpBypass = false;
    int32_t _meshEditorLeft;
    int32_t _meshEditorWidth;
    int32_t _meshEditorHeight;

	std::shared_ptr<UiControlContainer> _spFixedControlsPanel;
	std::shared_ptr<UiControlContainer> _spCornersControlPanel;
	std::shared_ptr<UiControlContainer> _spArbitraryControlsPanel;
    std::shared_ptr<UiControlContainer> _spFisheyeControlsPanel;
	std::shared_ptr<UiControlContainer> _spMeshEditorPanel;
	std::shared_ptr<UiControlContainer> _spWarpMiscPanel;
	std::shared_ptr<UiControlContainer> _spModeContainerPanel;
	std::shared_ptr<UiControlContainer> _spWarningsPanel;

    bool _enableDebugControls;
};

using EasyWarpRotationCB = std::function<void(SwApi::WarpAdapter::EasyWarpRotation)>;

class WarpEasyControls : public IpUiControls
{
public:
    WarpEasyControls(std::shared_ptr<SwApi::WarpAdapter> spWarpAdapter,
					 CapturePictureCB captureCB = nullptr,
                     EasyWarpRotationCB easyWarpRotationCB = nullptr);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { return "WarpEasy"; }
    void EnableControls(bool state);
    void SetRotationControl(SwApi::WarpAdapter::EasyWarpRotation rotation);

private:
    std::shared_ptr<SwApi::WarpAdapter> _spWarpAdapter;
    std::shared_ptr<UiControlItemBoolean> _spHorizontalMirrorCheckbox;
    std::shared_ptr<UiControlItemEnum> _spRotationDropdown;
    CapturePictureCB _captureCB;
    EasyWarpRotationCB _easyWarpRotationCB;
};

class ShowWarpDialogUiUpdate : public UiUpdate
{
public:
    ShowWarpDialogUiUpdate(const std::vector<std::shared_ptr<UiControlContainer>>& controls,
                           uint32_t outputWidth, 
                           uint32_t outputHeight,
                           uint32_t client_id = allClients);
};
