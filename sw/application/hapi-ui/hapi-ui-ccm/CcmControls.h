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
#include "Ccm.h"
#include "CoeffGen.h"

/// Encapsulates UI functionality for the Black Level Statistics IP
class CcmControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given BLC instance.
    CcmControls(const std::string& name, 
                const std::shared_ptr<SwApi::Ccm>& spCcm,
                bool enableDebugUi = true);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    void GetColourStrengths(float& r, float& g, float& b);
    void SetColourStrengths(float r, float g, float b);

    std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }

private:
    void RecalculateResultantMatrix();

    const std::string _name;

    std::shared_ptr<UiControlItemSlider> _spHueSlider;
    std::shared_ptr<UiControlItemSlider> _spSatSlider;
    std::shared_ptr<UiControlItemSlider> _spValSlider;
    std::shared_ptr<UiControlItemBoolean> _spEnableTempBool;
    std::shared_ptr<UiControlItemSlider> _spTempSlider;
    std::shared_ptr<UiControlItemSlider> _spTintSlider;
    std::shared_ptr<UiControlItemSlider> _spRStrength;
    std::shared_ptr<UiControlItemSlider> _spGStrength;
    std::shared_ptr<UiControlItemSlider> _spBStrength;
    std::shared_ptr<UiControlItemSlider> _spContrastSlider;
    std::shared_ptr<UiControlItemSlider> _spImageFloorSlider;
    std::shared_ptr<UiControlItemSlider> _spSummScaleSlider;

    float _hue;
    float _sat;
    float _val;
    float _temperature;
    float _tint;
    float _rStrength;
    float _gStrength;
    float _bStrength;
    float _contrast;
    float _floor;

    bool _format; // True = RGB, False = BGR

    bool _shouldAdjustTemperature;

    // The resultant matrix is built up of a chain of many floating point
    // matrix multiplications. To save computation time, we store the pipeline
    // in stages, so that changes to lower-levels don't require recalculating
    // the entire set of matrices.
    vvp::ccm::FloatCoefficients _hsvMatrix;
    vvp::ccm::FloatCoefficients _temperatureMatrix;
    vvp::ccm::FloatCoefficients _strengthMatrix;
    vvp::ccm::FloatCoefficients _contrastMatrix;

    bool _updateHSV;
    bool _updateTemperature;
    bool _updateStrength;
    bool _updateContrastAndFloor;

    std::shared_ptr<SwApi::Ccm> _spCcm;

    bool _enableDebugUi = false;

    void ResetParamsAndRecalculate();

    // DEBUG

    int32_t _summand_scale = 10;

};
