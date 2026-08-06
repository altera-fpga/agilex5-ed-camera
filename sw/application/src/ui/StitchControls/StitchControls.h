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
#include "StitchAdapter.h"

class StitchControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    StitchControls(const std::shared_ptr<SwApi::StitchAdapter>& spStitchAdapter, bool powerUser = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "Stitch"; };

    void Update();

private:
    using tLensConfiguration = struct {
        std::string name;
        float hFoV;
        float dFoV;
        float focalLength;
        float distortionFTheta;
        bool distortionOverride;
        float distortionA;
        float distortionB;
        float distortionC;
        float distortionD;
    };

    static const int32_t _defaultOffsetX;
    static const int32_t _defaultOffsetY;
    static const uint32_t _defaultOverlap;
    static const  float _defaultLensInterCameraAngle;
    static const  float _defaultHFoV;
    static const  float _defaultDFoV;
    static const  float _defaultFocalLength;
    static const  float _defaultDistortionFTheta;

    static const std::vector<tLensConfiguration> _lensConfigurations;

    std::shared_ptr<SwApi::StitchAdapter>     _spStitchAdapter;
    bool _powerUser;

    std::shared_ptr<UiControlItemSlider>        _spOffsetX;
    std::shared_ptr<UiControlItemSlider>        _spOffsetY;
    std::shared_ptr<UiControlItemSlider>        _spOverlap;
    std::shared_ptr<UiControlItemEnum>          _spLensSelect;
    std::shared_ptr<UiControlItemFloat>         _spLensInterCameraAngle;
    std::shared_ptr<UiControlItemFloat>         _spHFoV;
    std::shared_ptr<UiControlItemFloat>         _spDFoV;
    std::shared_ptr<UiControlItemFloat>         _spFocalLength;
    std::shared_ptr<UiControlItemFloat>         _spDistortionFTheta;
    std::shared_ptr<UiControlItemLabel>         _spTranformDecriptionLabel;
    std::shared_ptr<UiControlItemLabel>         _spTranformFormulaLabel;
    std::shared_ptr<UiControlItemFloat>         _spDistortionA;
    std::shared_ptr<UiControlItemFloat>         _spDistortionB;
    std::shared_ptr<UiControlItemFloat>         _spDistortionC;
    std::shared_ptr<UiControlItemFloat>         _spDistortionD;
    std::shared_ptr<UiControlItemBoolean>       _spDirectTransform;
    std::shared_ptr<UiControlItemBoolean>       _spLensCorrection;
    std::shared_ptr<UiControlItemBoolean>       _spLensDistortion;
    std::shared_ptr<UiControlItemBoolean>       _spLensDistortionOverride;
    std::shared_ptr<UiControlItemBoolean>       _spCylindricalProjection;

    int32_t _offsetX;
    int32_t _offsetY;
    uint32_t _overlap;
    uint32_t _lensSelectIdx;
    float _lensInterCameraAngle;
    float _hfov;
    float _dfov;
    float _focalLength;
    float _distortionFTheta;
    float _distortion_a;
    float _distortion_b;
    float _distortion_c;
    float _distortion_d;
    bool _directTransform;
    bool _lensCorrection;
    bool _lensDistortion;
    bool _lensDistortionOverride;
    bool _cylindricalProjection;

    uint32_t _lensCustomIdx;
};
