/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspProfileColourCorrectionControls.h"
#include "WhiteBalance.h"
#include "UiElements.h"

#include "CoeffGen.h"

#include <string>
#include <memory>
#include <cstdlib>

static const int summand_scale = 10;
static const bool isRGB_or_BGR = true; // True = RGB, false = BGR


VvpIspProfileColourCorrectionControls::VvpIspProfileColourCorrectionControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                                                     const std::shared_ptr<WhiteBalanceController>& spWBController,
                                                                     const std::shared_ptr<SwApi::ICsc>& spCcm)
: _spProfile(spProfile),
  _spWBController(spWBController),
  _spCcm(spCcm)
{

}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileColourCorrectionControls::AddUiElements()
{
    auto spContainer = std::make_shared<UiControlContainer>("Color Correction", GetSettingsSectionName());

    auto matrixChangeCB = [this](uint32_t clientID, float& value)
    {
        if (!_spR0Slider ||
            !_spR1Slider ||
            !_spR2Slider ||
            !_spG0Slider ||
            !_spG1Slider ||
            !_spG2Slider ||
            !_spB0Slider ||
            !_spB1Slider ||
            !_spB2Slider)
        {
            return;
        }
        vvp::ccm::FloatCoefficients matrix = vvp::ccm::GenerateIdentityMatrix();

        matrix.A0 = _spR0Slider->GetValue<float>();
        matrix.B0 = _spR1Slider->GetValue<float>();
        matrix.C0 = _spR2Slider->GetValue<float>();

        matrix.A1 = _spG0Slider->GetValue<float>();
        matrix.B1 = _spG1Slider->GetValue<float>();
        matrix.C1 = _spG2Slider->GetValue<float>();

        matrix.A2 = _spB0Slider->GetValue<float>();
        matrix.B2 = _spB1Slider->GetValue<float>();
        matrix.C2 = _spB2Slider->GetValue<float>();

        intel_vvp_coefficients coeffs;

        if (isRGB_or_BGR)
        {
            // If RGB, then the matrix is set up as how the coeff gen library expects it
            coeffs.coeffs[0].c1 = matrix.A0;
            coeffs.coeffs[0].c2 = matrix.B0;
            coeffs.coeffs[0].c3 = matrix.C0;
            coeffs.s[0]         = matrix.S0;

            coeffs.coeffs[2].c1 = matrix.A2;
            coeffs.coeffs[2].c2 = matrix.B2;
            coeffs.coeffs[2].c3 = matrix.C2;
            coeffs.s[2]         = matrix.S2;
        }
        else
        {
            // If BGR, then we need to swap the first two rows of the coefficient matrix
            coeffs.coeffs[2].c1 = matrix.A0;
            coeffs.coeffs[2].c2 = matrix.B0;
            coeffs.coeffs[2].c3 = matrix.C0;
            coeffs.s[2]         = matrix.S0;

            coeffs.coeffs[0].c1 = matrix.A2;
            coeffs.coeffs[0].c2 = matrix.B2;
            coeffs.coeffs[0].c3 = matrix.C2;
            coeffs.s[0]         = matrix.S2;
        }


        coeffs.coeffs[1].c1 = matrix.A1;
        coeffs.coeffs[1].c2 = matrix.B1;
        coeffs.coeffs[1].c3 = matrix.C1;
        coeffs.s[1]         = matrix.S1;

        _spCcm->ApplyCoefficientMatrix(&coeffs, summand_scale, _spCcm->GetInputColorSpace(), _spCcm->GetOutputColorSpace());
    };

    _spR0Slider = spContainer->AddSliderControl("R : Rmod", -1.0, 3.0, matrixChangeCB, "ccmRRslider", 1.0, 3);
    _spR1Slider = spContainer->AddSliderControl("R : Gmod", -1.0, 3.0, matrixChangeCB, "ccmRGslider", 0.0, 3);
    _spR2Slider = spContainer->AddSliderControl("R : Bmod", -1.0, 3.0, matrixChangeCB, "ccmRBslider", 0.0, 3);

    spContainer->AddSeparator();

    _spG0Slider = spContainer->AddSliderControl("G : Rmod", -1.0, 3.0, matrixChangeCB, "ccmGRslider", 0.0, 3);
    _spG1Slider = spContainer->AddSliderControl("G : Gmod", -1.0, 3.0, matrixChangeCB, "ccmGGslider", 1.0, 3);
    _spG2Slider = spContainer->AddSliderControl("G : Bmod", -1.0, 3.0, matrixChangeCB, "ccmGBslider", 0.0, 3);

    spContainer->AddSeparator();

    _spB0Slider = spContainer->AddSliderControl("B : Rmod", -1.0, 3.0, matrixChangeCB, "ccmBRslider", 0.0, 3);
    _spB1Slider = spContainer->AddSliderControl("B : Gmod", -1.0, 3.0, matrixChangeCB, "ccmBGslider", 0.0, 3);
    _spB2Slider = spContainer->AddSliderControl("B : Bmod", -1.0, 3.0, matrixChangeCB, "ccmBBslider", 1.0, 3);

    return {std::move(spContainer)};
}

void VvpIspProfileColourCorrectionControls::UpdateUiWithCurrentProfile()
{

}
