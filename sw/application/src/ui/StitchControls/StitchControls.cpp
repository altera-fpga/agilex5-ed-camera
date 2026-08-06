/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "StitchControls.h"
#include "UiElements.h"

#include <string>

const int32_t StitchControls::_defaultOffsetX = 0;
const int32_t StitchControls::_defaultOffsetY = 0;
const uint32_t StitchControls::_defaultOverlap = 100;
const  float StitchControls::_defaultLensInterCameraAngle(90.0);
const  float StitchControls::_defaultHFoV(110.0);
const  float StitchControls::_defaultDFoV(127.0);
const  float StitchControls::_defaultFocalLength(0.004);
const  float StitchControls::_defaultDistortionFTheta(-0.0018);

const std::vector<StitchControls::tLensConfiguration> StitchControls::_lensConfigurations =
{
    {"Framos 110 (FLP- AM-040-02-V-00)", 110.0, 127.0, 0.004, -0.0018, true, 3.998537, -0.03203, 0.02614, -0.00582},
    {"Sunex 190 (DSL315B-650-F2.3)", 190.0, 190.0, 0.00267, -0.18, false, 0.0, 0.0, 0.0, 0.0},
    {"Custom", 0.0, 0.0, 0.0, 0.0, false, 0.0, 0.0, 0.0, 0.0},
};

StitchControls::StitchControls(const std::shared_ptr<SwApi::StitchAdapter>& spStitchAdapter, bool powerUser)
: _spStitchAdapter(spStitchAdapter)
, _powerUser(powerUser)
, _offsetX(_defaultOffsetX)
, _offsetY(_defaultOffsetY)
, _overlap(_defaultOverlap)
, _lensSelectIdx(0)
, _lensInterCameraAngle(_defaultLensInterCameraAngle)
, _hfov(_defaultHFoV)
, _dfov(_defaultDFoV)
, _focalLength(_defaultFocalLength)
, _distortionFTheta(_defaultDistortionFTheta)
, _lensCustomIdx(2)
, _directTransform(true)
, _lensCorrection(true)
, _lensDistortion(true)
, _lensDistortionOverride(false)
, _cylindricalProjection(true)
{
}

std::vector<std::shared_ptr<UiControlContainer>> StitchControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Stitch", GetSettingsSectionName());

    if(_spStitchAdapter)
    {
        _spStitchAdapter->PauseUpdates(true);
    }

    auto offsetXCB = [this](uint32_t clientID, int32_t& val)
    {
        _offsetX = val;
        _spStitchAdapter->SetOffsetX(_offsetX);
    };
    _spOffsetX = spContainer->AddSliderControl("Offset X", -100, 100, offsetXCB, "Offset X", _offsetX);

    auto offsetYCB = [this](uint32_t clientID, int32_t& val)
    {
        _offsetY = val;
        _spStitchAdapter->SetOffsetY(_offsetY);
    };
    _spOffsetY = spContainer->AddSliderControl("Offset Y", -100, 100, offsetYCB, "Offset Y", _offsetY);

    auto overlapCB = [this](uint32_t clientID, int32_t& val)
    {
        _overlap = (uint32_t)val;
        _spStitchAdapter->SetOverlap(_overlap);
    };
    _spOverlap = spContainer->AddSliderControl("Stich over (<= Offset X)", 0, 256, overlapCB, "Overlap", _overlap);
 
    std::vector<UiEnumOption> uiLensSelectEnumOpts{};

    for(uint32_t lens = 0; lens < _lensConfigurations.size(); lens++)
    {
        uiLensSelectEnumOpts.emplace_back(_lensConfigurations[lens].name, lens);
    }
    _lensCustomIdx = _lensConfigurations.size() - 1;

    auto uiLensSelectCb = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t){
        _lensSelectIdx = selected._userItemData;
        auto lens = _lensConfigurations[_lensSelectIdx];
        if(lens.name != "Custom")
        {
            _hfov = lens.hFoV;
            _dfov = lens.dFoV;
            _focalLength = lens.focalLength;
            _distortionFTheta = lens.distortionFTheta;
            _lensDistortionOverride = lens.distortionOverride;

            if(_spLensDistortionOverride)
            {
                _spLensDistortionOverride->UpdateValue(_lensDistortionOverride, true);
            }
            if(_spHFoV)
            {
                _spHFoV->UpdateValue(_hfov, true);
            }
            if(_spDFoV)
            {
                _spDFoV->UpdateValue(_dfov, true);
            }
            if(_spFocalLength)
            {
                _spFocalLength->UpdateValue(_focalLength*1000.0, true);
            }
            if(_spDistortionFTheta)
            {
                _spDistortionFTheta->UpdateValue(_distortionFTheta*100.0, true);
            }
            if(_spDistortionA && _spDistortionB && _spDistortionC && _spDistortionD)
            {
                if(lens.distortionOverride)
                {
                    _distortion_a = lens.distortionA/1000.0;
                    _distortion_b = lens.distortionB/1000.0;
                    _distortion_c = lens.distortionC/1000.0;
                    _distortion_d = lens.distortionD/1000.0;
                    _spStitchAdapter->SetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
                }
                else
                {
                    _spStitchAdapter->GetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
                }
                _spDistortionA->UpdateValue(_distortion_a*1000.0);
                _spDistortionB->UpdateValue(_distortion_b*1000.0);
                _spDistortionC->UpdateValue(_distortion_c*1000.0);
                _spDistortionD->UpdateValue(_distortion_d*1000.0);
            }
        }
    };

    _spLensSelect = spContainer->AddEnumControl("Lens Select",
                                                uiLensSelectEnumOpts,
                                                uiLensSelectCb,
                                                "LensSelect",
                                                0);

    auto lensInterCameraAngleCB = [this](uint32_t clientID, float& val)
    {
        _lensInterCameraAngle = val;
        _spStitchAdapter->SetInterCameraAngle(_lensInterCameraAngle);
     };
    _spLensInterCameraAngle = spContainer->AddFloatControl("Lens Inter-Camera Angle", 0.0, 180.0, lensInterCameraAngleCB, "LensInterCameraAngle", _lensInterCameraAngle);
    _spLensInterCameraAngle->ShowDecimalPlaces(1);
    _spLensInterCameraAngle->SetValuePreAndPostfix("", "degrees");
    
    auto hfovCB = [this](uint32_t clientID, float& val)
    {
        _hfov = val;
        _spStitchAdapter->SetHFoV(_hfov);
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_lensConfigurations[_lensSelectIdx].hFoV < (_hfov - 0.000000001)) ||
                   (_lensConfigurations[_lensSelectIdx].hFoV > (_hfov + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
     };
    _spHFoV = spContainer->AddFloatControl("HFoV", 0.0, 360.0, hfovCB, "HFoV", _hfov);
    _spHFoV->SetValuePreAndPostfix("", "degrees");
    
    auto dfovCB = [this](uint32_t clientID, float& val)
    {
        _dfov = val;
        _spStitchAdapter->SetDFoV(_dfov);
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_lensConfigurations[_lensSelectIdx].dFoV < (_dfov - 0.000000001)) ||
                   (_lensConfigurations[_lensSelectIdx].dFoV > (_dfov + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
     };
    _spDFoV = spContainer->AddFloatControl("DFoV", 0.0, 360.0, dfovCB, "DFoV", _dfov);
    _spDFoV->SetValuePreAndPostfix("", "degrees");
    
    auto focalLengthCB = [this](uint32_t clientID, float& val)
    {
        _focalLength = val/1000.0;
        _spStitchAdapter->SetFocalLength(_focalLength);
        if(_spDistortionA && _spDistortionB && _spDistortionC && _spDistortionD)
        {
            _spStitchAdapter->GetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
            _spDistortionA->UpdateValue(_distortion_a*1000.0);
            _spDistortionB->UpdateValue(_distortion_b*1000.0);
            _spDistortionC->UpdateValue(_distortion_c*1000.0);
            _spDistortionD->UpdateValue(_distortion_d*1000.0);
        }
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_lensConfigurations[_lensSelectIdx].focalLength < (_focalLength - 0.000000001)) ||
                   (_lensConfigurations[_lensSelectIdx].focalLength > (_focalLength + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
    };
    _spFocalLength = spContainer->AddFloatControl("FocalLength", 0.0, 100.0, focalLengthCB, "FocalLength", 1000.0*_focalLength);
    _spFocalLength->ShowDecimalPlaces(3);
    _spFocalLength->SetValuePreAndPostfix("", "mm");
    
    auto distortionFThetaCB = [this](uint32_t clientID, float& val)
    {
        _distortionFTheta = val/100.0f;
        _spStitchAdapter->SetDistortionFTheta(_distortionFTheta);
        if(_spDistortionA && _spDistortionB && _spDistortionC && _spDistortionD)
        {
            _spStitchAdapter->GetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
            _spDistortionA->UpdateValue(_distortion_a*1000.0);
            _spDistortionB->UpdateValue(_distortion_b*1000.0);
            _spDistortionC->UpdateValue(_distortion_c*1000.0);
            _spDistortionD->UpdateValue(_distortion_d*1000.0);
        }
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_lensConfigurations[_lensSelectIdx].distortionFTheta < (_distortionFTheta - 0.000000001)) ||
                   (_lensConfigurations[_lensSelectIdx].distortionFTheta > (_distortionFTheta + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
     };
    _spDistortionFTheta = spContainer->AddFloatControl("Distortion (F-Theta)", -100.0, 100.0, distortionFThetaCB, "DistortionFTheta", _distortionFTheta*100);
    _spDistortionFTheta->ShowDecimalPlaces(5);
    _spDistortionFTheta->SetValuePreAndPostfix("", "%");

    //_spTranformDecriptionLabel = spContainer->AddLabelControl("Displacement from sensor centre in meters calculated as:");
    _spTranformDecriptionLabel = spContainer->AddLabelControl("Displacement = ", "A*Theta + B*Theta^2 +");
    _spTranformFormulaLabel = spContainer->AddLabelControl(" ", "C*Theta^3 + D*Theta^4");

    // get current distortion settings
    _spStitchAdapter->GetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);

    auto distortion_a_CB = [this](uint32_t clientID, float& val)
    {
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_distortion_a < (val/1000.0 - 0.000000001)) ||
                   (_distortion_a > (val/1000.0 + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
        _distortion_a = val/1000.0;
        _spStitchAdapter->SetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
    };
    _spDistortionA = spContainer->AddFloatControl("Displacement: A", -1000.0, 1000.0, distortion_a_CB, "Distortion_A", _distortion_a*1000.0);
    _spDistortionA->ShowDecimalPlaces(5);
    _spDistortionA->SetValuePreAndPostfix("", "mm/rad");
    
    auto distortion_b_CB = [this](uint32_t clientID, float& val)
    {
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_distortion_b < (val/1000.0 - 0.000000001)) ||
                   (_distortion_b > (val/1000.0 + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
        _distortion_b = val/1000.0;
        _spStitchAdapter->SetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
    };
    _spDistortionB = spContainer->AddFloatControl("Displacement: B", -1000.0, 1000.0, distortion_b_CB, "Distortion_B", _distortion_b*1000.0);
    _spDistortionB->ShowDecimalPlaces(5);
    _spDistortionB->SetValuePreAndPostfix("", "mm/rad^2");
    
    auto distortion_c_CB = [this](uint32_t clientID, float& val)
    {
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_distortion_c < (val/1000.0 - 0.000000001)) ||
                   (_distortion_c > (val/1000.0 + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
        _distortion_c = val/1000.0;
        _spStitchAdapter->SetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
    };
    _spDistortionC = spContainer->AddFloatControl("Displacement: C", -1000.0, 1000.0, distortion_c_CB, "Distortion_C", _distortion_c*1000.0);
    _spDistortionC->ShowDecimalPlaces(5);
    _spDistortionC->SetValuePreAndPostfix("", "mm/rad^3");
    
    auto distortion_d_CB = [this](uint32_t clientID, float& val)
    {
        if(_spLensSelect)
        {
            if(_lensSelectIdx != _lensCustomIdx)
            {
                if((_distortion_d < (val/1000.0 - 0.000000001)) ||
                   (_distortion_d > (val/1000.0 + 0.000000001)))
                {
                    _spLensSelect->UpdateValue(_lensCustomIdx);
                }
            }
        }
        _distortion_d = val/1000.0;
        _spStitchAdapter->SetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
    };
    _spDistortionD = spContainer->AddFloatControl("Displacement: D", -1000.0, 1000.0, distortion_d_CB, "Distortion_D", _distortion_d*1000.0);
    _spDistortionD->ShowDecimalPlaces(5);
    _spDistortionD->SetValuePreAndPostfix("", "mm/rad^4");
    
    auto directTransformCB = [this](uint32_t clientID, bool value)
    {
        _directTransform = value;
        _spStitchAdapter->SetDirectTransform(_directTransform);
        if(_directTransform)
        {
            if(_spLensCorrection)
            {
                _spLensCorrection->Enable(false);
            }
            if(_spLensDistortion)
            {
                _spLensDistortion->Enable(true);
            }
            if(_spCylindricalProjection)
            {
                _spCylindricalProjection->Enable(false);
            }
        }
        else
        {
            if(_spLensCorrection)
            {
                _spLensCorrection->Enable(true);
            }
            if(_spCylindricalProjection)
            {
                _spCylindricalProjection->Enable(true);
            }
            if(_spLensDistortion)
            {
                if(_lensCorrection)
                {
                    _spLensDistortion->Enable(true);
                }
                else
                {
                    _spLensDistortion->Enable(false);
                }
            }
        }
    };
    _spDirectTransform = spContainer->AddBoolControl("Direct Transform (Lens correction + cylindrical projection)", directTransformCB, "directTransform", true);    

    auto lensCorrectionCB = [this](uint32_t clientID, bool value)
    {
        _lensCorrection = value;
        _spStitchAdapter->SetLensCorrection(_lensCorrection);
        if(_spLensDistortion)
        {
            if(_lensCorrection)
            {
                _spLensDistortion->Enable(true);
            }
            else
            {
                _spLensDistortion->Enable(false);
            }
        }
    };
    _spLensCorrection = spContainer->AddBoolControl("Lens Correction", lensCorrectionCB, "lensCorrection", true);

    auto lensDistortionCB = [this](uint32_t clientID, bool value)
    {
        _lensDistortion = value;
        _spStitchAdapter->SetLensDistortion(_lensDistortion);
    };
    _spLensDistortion = spContainer->AddBoolControl("Lens Distortion", lensDistortionCB, "lensDistortion", true);

    auto lensDistortionOverrideCB = [this](uint32_t clientID, bool value)
    {
        _lensDistortionOverride = value;
        _spStitchAdapter->SetLensDistortionOverride(_lensDistortionOverride);
        if(_lensDistortionOverride)
        {
            _spDistortionA->Enable(true);
            _spDistortionB->Enable(true);
            _spDistortionC->Enable(true);
            _spDistortionD->Enable(true);
        }
        else
        {
            _spDistortionA->Enable(false);
            _spDistortionB->Enable(false);
            _spDistortionC->Enable(false);
            _spDistortionD->Enable(false);
        }
    };
    _spLensDistortionOverride = spContainer->AddBoolControl("Lens Distortion Override", lensDistortionOverrideCB, "lensDistortionOverride", false);

    auto cylindricalProjectionCB = [this](uint32_t clientID, bool value)
    {
        _cylindricalProjection = value;
        _spStitchAdapter->SetCylindricalProjection(_cylindricalProjection);
    };
    _spCylindricalProjection = spContainer->AddBoolControl("Cylindrical Projection", cylindricalProjectionCB, "cylindricalProjection", true);

    // Initialise values
    if(_spLensSelect)
    {
        auto index = _spLensSelect->GetSelectedIndex();
        _spLensSelect->UpdateValue(index, true);
    }


    if(_spDistortionA && _spDistortionB && _spDistortionC && _spDistortionD)
    {
        _spStitchAdapter->GetLensDistortionCoef(_distortion_a, _distortion_b, _distortion_c, _distortion_d);
        _spDistortionA->UpdateValue(_distortion_a*1000.0);
        _spDistortionB->UpdateValue(_distortion_b*1000.0);
        _spDistortionC->UpdateValue(_distortion_c*1000.0);
        _spDistortionD->UpdateValue(_distortion_d*1000.0);
        if(_lensDistortionOverride)
        {
            _spDistortionA->Enable(true);
            _spDistortionB->Enable(true);
            _spDistortionC->Enable(true);
            _spDistortionD->Enable(true);
        }
        else
        {
            _spDistortionA->Enable(false);
            _spDistortionB->Enable(false);
            _spDistortionC->Enable(false);
            _spDistortionD->Enable(false);
        }
    }

    if(_directTransform)
    {
        _spLensCorrection->Enable(false);
        _spCylindricalProjection->Enable(false);
        _spLensDistortion->Enable(true);
    }
    else
    {
        _spLensCorrection->Enable(true);
        _spCylindricalProjection->Enable(true);
        if(_lensCorrection)
        {
            _spLensDistortion->Enable(true);
        }
        else
        {
            _spLensDistortion->Enable(false);
        }
    }
    if(_spStitchAdapter)
    {
        _spStitchAdapter->PauseUpdates(false);
    }

    auto resetButtonCB = [this](uint32_t clientID)
    {
        if(_spStitchAdapter)
        {
            _spStitchAdapter->PauseUpdates(true);
        }
        _spOffsetX->UpdateValue(_defaultOffsetX, true);
        _spOffsetY->UpdateValue(_defaultOffsetY, true);
        _spOverlap->UpdateValue((int32_t)_defaultOverlap, true);
        _spLensInterCameraAngle->UpdateValue(_defaultLensInterCameraAngle, true);
        _spLensSelect->UpdateValue(0, true);
        
        if(_spDirectTransform)
        {
            _spDirectTransform->UpdateValue(true, true);
        }
        if(_spLensCorrection)
        {
            _spLensCorrection->UpdateValue(true, true);
        }
        if(_spLensDistortion)
        {
            _spLensDistortion->UpdateValue(true, true);
        }
        if(_spCylindricalProjection)
        {
            _spCylindricalProjection->UpdateValue(true, true);
        }
        if(_spLensDistortionOverride)
        {
            _spLensDistortionOverride->UpdateValue(false, true);
        }

        if(_spStitchAdapter)
        {
            _spStitchAdapter->SetOverlap(_overlap, true);
            _spStitchAdapter->PauseUpdates(false);
        }
    };

    spContainer->AddHeaderButtons( {
        { "CornerControlsReset", resetButtonCB, "Reset", "OJL/Images/Reset.png" }
    });


    return std::vector<std::shared_ptr<UiControlContainer>>{spContainer};
}

void StitchControls::Update()
{
    if(_spOverlap && _spStitchAdapter)
    {
        int32_t overlapMax = (int32_t)_spStitchAdapter->GetLayerOverlap();
        int32_t currentOverlap = _spOverlap->GetValue<int32_t>();
        if(currentOverlap > overlapMax)
        {
            _spOverlap->UpdateValue(overlapMax, true);
        }
        _spOverlap->UpdateRange(0, overlapMax);
    }
}
