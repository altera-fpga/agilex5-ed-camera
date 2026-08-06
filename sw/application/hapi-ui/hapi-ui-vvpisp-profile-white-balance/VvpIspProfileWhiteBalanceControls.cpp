/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspProfileWhiteBalanceControls.h"
#include "WhiteBalance.h"
#include "UiElements.h"
#include "CoeffGen.h"

#include <string>
#include <memory>
#include <cstdlib>


static const bool is_RGB = true; // Demo uses BGR // FIXME - make this global somewhere


VvpIspProfileWhiteBalanceControls::VvpIspProfileWhiteBalanceControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                                                     const std::shared_ptr<WhiteBalanceController>& spWBController,
                                                                     const std::shared_ptr<SwApi::Wbc>& spWhiteBalanceCorrection,
                                                                     const std::shared_ptr<SwApi::Ccm>& spCcm)
: _spProfile(spProfile),
  _spWBController(spWBController),
  _spWhiteBalanceCorrection(spWhiteBalanceCorrection),
  _spColourCorrectionMatrix(spCcm)
{

}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileWhiteBalanceControls::AddUiElements()
{
    auto spContainer = std::make_shared<UiControlContainer>("White Level Calibration", GetSettingsSectionName());

    /*
    // TODO - uncomment when we're done with the demo!
    spContainer->AddLabelControl("Luma Scaling TBD");

    auto lumSelectCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
    {

    };

    std::vector<UiEnumOption> lumVec;

    _spLumSelectEnum = spContainer->AddEnumControl(
        "Select Luminance", lumVec, lumSelectCB, "lumSelectDropdown", 0);

    auto addLumCB = [this](uint32_t clientID)
    {

    };
    _spLumAddButton = spContainer->AddButtonControl("Add Current Luminance", addLumCB);

    auto delLumCB = [this](uint32_t clientID)
    {

    };
    _spLumDelButton = spContainer->AddButtonControl("Delete Selected Luminance", delLumCB);

    spContainer->AddSeparator();

    spContainer->AddLabelControl("Luminance-Scaled Baseline");

    auto baseColTempCB = [this](uint32_t clientID, uint32_t& value)
    {

    };
    _spLumBaselineInt = spContainer->AddUIntegerControl("Baseline Colour Temp", 6500, 1000, 21000, baseColTempCB, false, 1);
    spContainer->AddLabelControl(""); // A separator but blank

    auto baseColSclChangeCB = [this](uint32_t clientID, uint32_t& value)
    {

    };
    _spLumBaseCfa00Int = spContainer->AddUIntegerControl("cfa_00 Scalar: ", 0, 0, 16394, baseColSclChangeCB, false, 1);
    _spLumBaseCfa01Int = spContainer->AddUIntegerControl("cfa_01 Scalar: ", 0, 0, 16394, baseColSclChangeCB, false, 1);
    _spLumBaseCfa10Int = spContainer->AddUIntegerControl("cfa_10 Scalar: ", 0, 0, 16394, baseColSclChangeCB, false, 1);
    _spLumBaseCfa11Int = spContainer->AddUIntegerControl("cfa_11 Scalar: ", 0, 0, 16394, baseColSclChangeCB, false, 1);

    auto recordBaselineCB = [this](uint32_t clientID)
    {

    };

    _spLumRecordButton = spContainer->AddButtonControl("Record", recordBaselineCB);

    // TODO - When the new luminance system is in place, implement it here and remove this

    _spLumSelectEnum->Enable(false);
    _spLumAddButton->Enable(false);
    _spLumDelButton->Enable(false);
    _spLumBaselineInt->Enable(false);
    _spLumBaseCfa00Int->Enable(false);
    _spLumBaseCfa01Int->Enable(false);
    _spLumBaseCfa10Int->Enable(false);
    _spLumBaseCfa11Int->Enable(false);
    _spLumRecordButton->Enable(false);

    spContainer->AddSeparator();
    */


    spContainer->AddLabelControl("Color Temperature Selection");

    auto wbcColTempCB = [this](uint32_t clientID, uint32_t& value)
    {
        _enteredTemp = value;
    };
    _spColTempInt = spContainer->AddUIntegerControl("Color Temp", 6500, 1000, 21000, wbcColTempCB, false, 1);

    auto colTempSelectCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
    {
        SelectTemp(std::stoi(selected._label), TCfaPhase::RGGB);
    };

    std::vector<UiEnumOption> colTempVec;

    _spColTempSelectEnum = spContainer->AddEnumControl(
        "Select Temp", colTempVec, colTempSelectCB, "colTempDropdown", 0);

    auto addTempCB = [this](uint32_t clientID)
    {
        // Create the entry in the table
        _spProfile->FindOrCreateEntryByColourTemp(_enteredTemp);
        // Now add to the list and select it
        ResetTempDropdownAndSelectTemp(_enteredTemp);

        if (_fpUpdateOverviewUiCB)
        {
            _fpUpdateOverviewUiCB();
        }
    };
    _spAddTempButton = spContainer->AddButtonControl("Add New Temp", addTempCB);

    auto delTempCB = [this](uint32_t clientID)
    {
        _spProfile->DeleteEntryByTemp(_selectedTemp);
        ResetTempDropdownAndSelectTemp(-1);

        if (_fpUpdateOverviewUiCB)
        {
            _fpUpdateOverviewUiCB();
        }
    };
    _spDelTempButton = spContainer->AddButtonControl("Delete Selected", delTempCB);

    spContainer->AddSeparator();

    auto wbcRecordSclChangeCB = [this](uint32_t clientID, uint32_t& value)
    {
        //std::cout << "WBC Record: cfa val changed\n";
        uint32_t wbcParams[4] = {_spColTempCfa00Int->GetValue(), _spColTempCfa01Int->GetValue(),
                                 _spColTempCfa10Int->GetValue(), _spColTempCfa11Int->GetValue()};
        float chanScalars[3];

        auto tempTableEntry = _spProfile->FindOrCreateEntryByColourTemp(_selectedTemp);

        switch (_spWhiteBalanceCorrection->GetCfaPhase())
        {
            case TCfaPhase::RGGB:
            {
                chanScalars[0] = wbcParams[0] / 2048.0f;
                chanScalars[1] = (wbcParams[1] + wbcParams[2]) / 4096.0f;
                chanScalars[2] = wbcParams[3] / 2048.0f;
                break;
            }
            case TCfaPhase::GRBG:
            {
                chanScalars[0] = wbcParams[1] / 2048.0f;
                chanScalars[1] = (wbcParams[0] + wbcParams[3]) / 4096.0f;
                chanScalars[2] = wbcParams[2] / 2048.0f;
                break;
            }
            case TCfaPhase::GBRG:
            {
                chanScalars[0] = wbcParams[2] / 2048.0f;
                chanScalars[1] = (wbcParams[0] + wbcParams[3]) / 4096.0f;
                chanScalars[2] = wbcParams[1] / 2048.0f;
                break;
            }
            case TCfaPhase::BGGR:
            {
                chanScalars[0] = wbcParams[3] / 2048.0f;
                chanScalars[1] = (wbcParams[1] + wbcParams[2]) / 4096.0f;
                chanScalars[2] = wbcParams[0] / 2048.0f;
                break;
            }
        }

        std::memcpy(tempTableEntry->_ChanScalars, chanScalars, sizeof(float) * 3);

        _spColTempCfa00Int->UpdateValue(wbcParams[0]);
        _spColTempCfa01Int->UpdateValue(wbcParams[1]);
        _spColTempCfa10Int->UpdateValue(wbcParams[2]);
        _spColTempCfa11Int->UpdateValue(wbcParams[3]);

        _spWhiteBalanceCorrection->SetAllColorScalers(wbcParams);
    };
    _spColTempCfa00Int = spContainer->AddUIntegerControl("cfa_00 Scalar: ", 0, 0, 16394, wbcRecordSclChangeCB, false, 1);
    _spColTempCfa01Int = spContainer->AddUIntegerControl("cfa_01 Scalar: ", 0, 0, 16394, wbcRecordSclChangeCB, false, 1);
    _spColTempCfa10Int = spContainer->AddUIntegerControl("cfa_10 Scalar: ", 0, 0, 16394, wbcRecordSclChangeCB, false, 1);
    _spColTempCfa11Int = spContainer->AddUIntegerControl("cfa_11 Scalar: ", 0, 0, 16394, wbcRecordSclChangeCB, false, 1);

    auto recordColourTempCB = [this](uint32_t clientID)
    {

        uint32_t wbcParams[4];

        _spWBController->ReadWBSAndUpdateProfile(_cfaPhase, _selectedTemp, wbcParams);

        _spColTempCfa00Int->UpdateValue(wbcParams[0]);
        _spColTempCfa01Int->UpdateValue(wbcParams[1]);
        _spColTempCfa10Int->UpdateValue(wbcParams[2]);
        _spColTempCfa11Int->UpdateValue(wbcParams[3]);

        _spWhiteBalanceCorrection->SetAllColorScalers(wbcParams);

    };

    _spReadColTempDataButton = spContainer->AddButtonControl("Read Color Temp Data", recordColourTempCB);


    _spColTempCfa00Int->Enable(false);
    _spColTempCfa01Int->Enable(false);
    _spColTempCfa10Int->Enable(false);
    _spColTempCfa11Int->Enable(false);
    _spReadColTempDataButton->Enable(false);

    spContainer->AddSeparator();
    spContainer->AddLabelControl("Color Correction");
    spContainer->AddLabelControl("");
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

        float matrix[3][4] = {0};

        matrix[0][0] = _spR0Slider->GetValue<float>();
        matrix[0][1] = _spG0Slider->GetValue<float>();
        matrix[0][2] = _spB0Slider->GetValue<float>();


        matrix[1][0] = _spR1Slider->GetValue<float>();
        matrix[1][1] = _spG1Slider->GetValue<float>();
        matrix[1][2] = _spB1Slider->GetValue<float>();

        matrix[2][0] = _spR2Slider->GetValue<float>();
        matrix[2][1] = _spG2Slider->GetValue<float>();
        matrix[2][2] = _spB2Slider->GetValue<float>();

        _spProfile->UpdateCcmCoeffsForTemp(_selectedTemp, matrix);

        vvp::ccm::FloatCoefficients ccmMatrix = vvp::ccm::GenerateIdentityMatrix();

        ccmMatrix.A0 = matrix[0][0];
        ccmMatrix.A1 = matrix[0][1];
        ccmMatrix.A2 = matrix[0][2];

        ccmMatrix.B0 = matrix[1][0];
        ccmMatrix.B1 = matrix[1][1];
        ccmMatrix.B2 = matrix[1][2];

        ccmMatrix.C0 = matrix[2][0];
        ccmMatrix.C1 = matrix[2][1];
        ccmMatrix.C2 = matrix[2][2];

        _spColourCorrectionMatrix->ApplySingularMatrix(ccmMatrix);
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

void VvpIspProfileWhiteBalanceControls::SelectTemp(uint16_t temp, TCfaPhase cfaPhase)
{
    _cfaPhase = cfaPhase;
    _selectedTemp = temp;

    if (_spProfile->IsColourTempRepresentableInTable(_selectedTemp) == EntryRepresentationStatus::Invalid)
    {
        return;
    }

    uint32_t wbcParams[4];
    auto tempTableEntry = _spProfile->GetInterpolatedTableEntryForTemperature(_selectedTemp);

    switch (_cfaPhase)
    {
        case TCfaPhase::RGGB:
        {
            wbcParams[0] = tempTableEntry._ChanScalars[0] * 2048;
            wbcParams[1] = tempTableEntry._ChanScalars[1] * 2048;
            wbcParams[2] = wbcParams[1];
            wbcParams[3] = tempTableEntry._ChanScalars[2] * 2048;
            break;
        }
        case TCfaPhase::GRBG:
        {
            wbcParams[0] = tempTableEntry._ChanScalars[1] * 2048;
            wbcParams[1] = tempTableEntry._ChanScalars[0] * 2048;
            wbcParams[2] = tempTableEntry._ChanScalars[2] * 2048;
            wbcParams[3] = wbcParams[0];
            break;
        }
        case TCfaPhase::GBRG:
        {
            wbcParams[0] = tempTableEntry._ChanScalars[1] * 2048;
            wbcParams[1] = tempTableEntry._ChanScalars[2] * 2048;
            wbcParams[2] = tempTableEntry._ChanScalars[0] * 2048;
            wbcParams[3] = wbcParams[0];
            break;
        }
        case TCfaPhase::BGGR:
        {
            wbcParams[0] = tempTableEntry._ChanScalars[2] * 2048;
            wbcParams[1] = tempTableEntry._ChanScalars[1] * 2048;
            wbcParams[2] = wbcParams[1];
            wbcParams[3] = tempTableEntry._ChanScalars[0] * 2048;
            break;
        }
    }

    _spColTempCfa00Int->UpdateValue(wbcParams[0]);
    _spColTempCfa01Int->UpdateValue(wbcParams[1]);
    _spColTempCfa10Int->UpdateValue(wbcParams[2]);
    _spColTempCfa11Int->UpdateValue(wbcParams[3]);

    _spWhiteBalanceCorrection->SetAllColorScalers(wbcParams);

    if (tempTableEntry._HasCcm)
    {
        //if (is_RGB)
        {
            _spR0Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[0][0]);
            _spG0Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[0][1]);
            _spB0Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[0][2]);

            _spR1Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[1][0]);
            _spG1Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[1][1]);
            _spB1Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[1][2]);

            _spR2Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[2][0]);
            _spG2Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[2][1]);
            _spB2Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[2][2]);
        }
        /*
        else
        {
            _spR0Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[2][0]);
            _spR1Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[2][1]);
            _spR2Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[2][2]);
            _spB0Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[0][0]);
            _spB1Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[0][1]);
            _spB2Slider->UpdateValue((float)tempTableEntry._CcmCoeffs[0][2]);
        }
        */
    }
    else
    {
        _spR0Slider->UpdateValue(1.0f);
        _spR1Slider->UpdateValue(0.f);
        _spR2Slider->UpdateValue(0.f);

        _spG0Slider->UpdateValue(0.f);
        _spG1Slider->UpdateValue(1.0f);
        _spG2Slider->UpdateValue(0.f);

        _spB0Slider->UpdateValue(0.f);
        _spB1Slider->UpdateValue(0.f);
        _spB2Slider->UpdateValue(1.0f);
    }
}

void VvpIspProfileWhiteBalanceControls::ResetTempDropdownAndSelectTemp(uint16_t temp)
{
    int tempSelectLength = _spColTempSelectEnum->GetLength(); // Because it gets smaller and smaller in each loop iteration
    int selectedIndex = 0;

    //std::cout << "GainSelect length: " << gainSelectLength << "\n";

    // Update the gain selection list
    // First we need to clear all the items
    for (int i = 0; i < tempSelectLength; i++)
    {
        //std::cout << "Removing index " << 0 << ": " << _spColTempSelectEnum->GetOption(0)._label << "\n";
        // 0 is because the list resizes, so we just remove from the top until they're all gone
        _spColTempSelectEnum->RemoveEnumItem(0, _spColTempSelectEnum->GetOption(0)._label);
    }

    std::vector<uint16_t> temps = _spProfile->GetListOfTemps();
    //std::cout << "gain length: " << temps.size() << "\n";
    if (temps.size() == 0)
    {
        _spColTempSelectEnum->Enable(false);
        _spColTempCfa00Int->Enable(false);
        _spColTempCfa01Int->Enable(false);
        _spColTempCfa10Int->Enable(false);
        _spColTempCfa11Int->Enable(false);
        _spReadColTempDataButton->Enable(false);
        return;
    }
    else
    {
        _spColTempSelectEnum->Enable(true);
        _spColTempCfa00Int->Enable(true);
        _spColTempCfa01Int->Enable(true);
        _spColTempCfa10Int->Enable(true);
        _spColTempCfa11Int->Enable(true);
        _spReadColTempDataButton->Enable(true);
    }

    for (int i = 0; i < (int)temps.size(); i++)
    {
        if (temps[i] == temp)
        {
            selectedIndex = i;
        }

        _spColTempSelectEnum->AddEnumItem(std::to_string(temps[i]), i);
    }

    _spColTempSelectEnum->UpdateValue(selectedIndex, true);
}

void VvpIspProfileWhiteBalanceControls::UpdateUiWithCurrentProfile()
{
    ResetTempDropdownAndSelectTemp(0);
}

void VvpIspProfileWhiteBalanceControls::LoadFunctionPointerForUpdatingOverviewUI(std::function<void(void)> fpUpdateOverviewUiCB)
{
    _fpUpdateOverviewUiCB = std::move(fpUpdateOverviewUiCB);
}
