/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <string>
#include "VvpIspProfileBlackLevelControls.h"
#include "UiElements.h"
#include "AnrUtils.h"



VvpIspProfileBlackLevelControls::VvpIspProfileBlackLevelControls(const std::shared_ptr<IFrameCapture>& spIFrameCapture,
                                                                 const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                                                 const std::shared_ptr<WhiteBalanceController>& spWBController,
                                                                 const ICameraPtr& spCamera,
                                                                 const std::shared_ptr<SwApi::Bls>& spBlackLevelStats,
                                                                 const std::shared_ptr<SwApi::Anr>& spAnr):
  _spIFrameCapture{spIFrameCapture},
  _spProfile{spProfile},
  _spWBController{spWBController},
  _spCamera{spCamera},
  _spBlackLevelStats{spBlackLevelStats},
  _spAnr{spAnr}
{
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileBlackLevelControls::AddUiElements()
{
    auto spContainer = std::make_shared<UiControlContainer>("Black Level Calibration", GetSettingsSectionName());

    spContainer->AddLabelControl("Analog Gain");

    auto lumSelectCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
    {
        SelectGain(std::stof(selected._label));
    };

    std::vector<UiEnumOption> lumVec;

    _spGainSelectEnum = spContainer->AddEnumControl(
        "Select Gain", lumVec, lumSelectCB, "lumSelectDropdown", 0);

    auto gainInputFloatCB = [this](uint32_t clientID, float& value) -> void
    {
        _enteredGain = value;
    };
    spContainer->AddFloatControl("Gain: ", 1, 1, 15.99, gainInputFloatCB);

    auto addGainCB = [this](uint32_t clientID)
    {
        AddNewGain(_enteredGain);
        if (_fpUpdateOverviewUiCB)
        {
            _fpUpdateOverviewUiCB();
        }
    };
    spContainer->AddButtonControl("Add New Gain", addGainCB);

    auto delGainCB = [this](uint32_t clientID)
    {
        DeleteGain(_selectedGain);
        if (_fpUpdateOverviewUiCB)
        {
            _fpUpdateOverviewUiCB();
        }
    };
    spContainer->AddButtonControl("Delete Selected Gain", delGainCB);

    spContainer->AddSeparator();

    spContainer->AddLabelControl("BLC Parameters");

    auto blackSclPdlChangeCB = [this](uint32_t clientID, uint32_t& value)
    {
        uint32_t cfa[4] = {
            _spBLCtrl_Ped00Int->GetValue(), _spBLCtrl_Ped01Int->GetValue(),
            _spBLCtrl_Ped10Int->GetValue(), _spBLCtrl_Ped11Int->GetValue()
        };

        uint32_t scalars[4] = {
            _spBLCtrl_Scl00Int->GetValue(), _spBLCtrl_Scl01Int->GetValue(),
            _spBLCtrl_Scl10Int->GetValue(), _spBLCtrl_Scl11Int->GetValue()
        };

        _spProfile->UpdateBlcParametersForGain(_selectedGain, cfa, scalars);
        _spWBController->SetGain(_selectedGain);
        _spWBController->ApplyBlackLevel();
    };
    _spBLCtrl_Ped00Int = spContainer->AddUIntegerControl("cfa_00 Pedestal: ", 0, 0, 1023, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Ped01Int = spContainer->AddUIntegerControl("cfa_01 Pedestal: ", 0, 0, 1023, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Ped10Int = spContainer->AddUIntegerControl("cfa_10 Pedestal: ", 0, 0, 1023, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Ped11Int = spContainer->AddUIntegerControl("cfa_11 Pedestal: ", 0, 0, 1023, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Scl00Int = spContainer->AddUIntegerControl("cfa_00 Scalar: ", 0, 0, 262143, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Scl01Int = spContainer->AddUIntegerControl("cfa_01 Scalar: ", 0, 0, 262143, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Scl10Int = spContainer->AddUIntegerControl("cfa_10 Scalar: ", 0, 0, 262143, blackSclPdlChangeCB, false, 1);
    _spBLCtrl_Scl11Int = spContainer->AddUIntegerControl("cfa_11 Scalar: ", 0, 0, 262143, blackSclPdlChangeCB, false, 1);

    auto readBLSCB = [this](uint32_t clientID)
    {
        ReadBLSAndUpdateUI();
    };

    _spReadBLSButton = spContainer->AddButtonControl("Read BLS", readBLSCB);

    auto macroReadBLSCB = [this](uint32_t clientID)
    {
        MacroControlReadBlackLevels();
    };

    _spMacroReadBLSButton = spContainer->AddButtonControl("Macro Read BLS", macroReadBLSCB);

    if (_spAnr)
    {
        spContainer->AddSeparator();
        // ANR bits
        uint32_t bps = _spBlackLevelStats->GetBps();

        auto combinedGainCB = [this](uint32_t clientID, float& val)
        {
            _combinedGain = val;
            _spProfile->UpdateAnrParametersForGain(_selectedGain, _darkNoise, _combinedGain);
            _spAnr->ApplyLuts(_combinedGain, _darkNoise);
        };
        _spAnrCombinedGain = spContainer->AddFloatControl("Combined Gain: ", 0.0f, 1024.0f, combinedGainCB, "anrCombinedGain", 0.0f);

        auto darkNoiseCB = [this](uint32_t clientID, float& val)
        {
            _darkNoise = val;
            _spProfile->UpdateAnrParametersForGain(_selectedGain, _darkNoise, _combinedGain);
            _spAnr->ApplyLuts(_combinedGain, _darkNoise);
        };
        _spAnrDarkNoise = spContainer->AddFloatControl("Dark Noise: ", 0.0f, 1024.0f, darkNoiseCB, "anrDarkNoise", 0.0f);

        auto readDarkFrameCB = [this, bps](uint32_t clientID)
        {
            // read and store a dark frame
            if (_spIFrameCapture)
            {
                auto darkFrame = _spIFrameCapture->CaptureRawFrame(bps, false);
                _darkFrame = std::move(darkFrame._data);
            }
        };

        _spReadDarkFrameButton = spContainer->AddButtonControl("Read Dark Frame", readDarkFrameCB);

        auto readSceneFramesCB = [this, bps](uint32_t clientID)
        {
            // read and store two normal frames
            if (_spIFrameCapture)
            {
                _sceneFrames.clear();

                {
                    auto sceneFrame = _spIFrameCapture->CaptureRawFrame(bps, false);
                    _sceneFrames.emplace_back(std::move(sceneFrame._data));
                }
                
                usleep(1000);
                
                {
                    auto sceneFrame = _spIFrameCapture->CaptureRawFrame(bps, false);
                    _sceneFrames.emplace_back(std::move(sceneFrame._data));
                }
            }
        };

        _spReadSceneFramesButton = spContainer->AddButtonControl("Read Scene Frames", readSceneFramesCB);

        auto calculateAnrParamsCB = [this, bps](uint32_t clientID)
        {
            // Check we have a dark frame and scene frames

            if (_darkFrame.size() == 0 || (_sceneFrames.size() < 2))
                return;

            // Use stored dark and scene frames to calculate combined gain and dark noise
            std::pair<float, float> anrParams = SwApi::AnrUtils::NoiseEstimator(_darkFrame, _sceneFrames, bps);

            _combinedGain = anrParams.first;
            _darkNoise = anrParams.second;

            std::cout << "Combined Gain: " << _combinedGain << "\nDark noise with gain: " << _darkNoise << std::endl;

            // Show the values in the UI
            _spAnrCombinedGain->UpdateValue(_combinedGain);
            _spAnrDarkNoise->UpdateValue(_darkNoise);

            // Update profile and apply the coefficents to the ANR
            _spProfile->UpdateAnrParametersForGain(_selectedGain, _darkNoise, _combinedGain);
            _spAnr->ApplyLuts(_combinedGain, _darkNoise);
        };
        _spCalculateParamsButton = spContainer->AddButtonControl("Calculate Params", calculateAnrParamsCB);
    }

    return {std::move(spContainer)};
}

void VvpIspProfileBlackLevelControls::AddNewGain(float gain)
{
     // Create the entry in the table
    _spProfile->FindOrCreateEntryByGain(_enteredGain);
    // _spProfile->FindOrCreateGainScaleTempEntry(_enteredGain); TODO - replace with luminance
    // Now add to the list and select it
    ResetGainDropdownAndSelectGain(_enteredGain);
}

void VvpIspProfileBlackLevelControls::DeleteGain(float gain)
{
    _spProfile->DeleteEntryByGain(_selectedGain);
    // _spProfile->DeleteGainScaleEntryByGain(_selectedGain); TODO - replace with luminance
    ResetGainDropdownAndSelectGain(-1);
}

void VvpIspProfileBlackLevelControls::SelectGain(float gain)
{
    _selectedGain = gain;

    // FIXME - replace with callout to AE camera system
    if(_spCamera)
        _spCamera->SetAnalogueGain(gain);

    if (_spProfile->IsGainRepresentableInTable(gain) == EntryRepresentationStatus::Invalid)
    {
        return;
    }

    AutoWhiteBalanceGainTableEntry entry = _spProfile->GetInterpolatedTableEntryForGain(gain);

   // _spMainCtrl_GainFloat->UpdateValue(gain);

    _spBLCtrl_Ped00Int->UpdateValue(entry._BlcPedestals[0]);
    _spBLCtrl_Ped01Int->UpdateValue(entry._BlcPedestals[1]);
    _spBLCtrl_Ped10Int->UpdateValue(entry._BlcPedestals[2]);
    _spBLCtrl_Ped11Int->UpdateValue(entry._BlcPedestals[3]);

    _spBLCtrl_Scl00Int->UpdateValue(entry._BlcScalars[0]);
    _spBLCtrl_Scl01Int->UpdateValue(entry._BlcScalars[1]);
    _spBLCtrl_Scl10Int->UpdateValue(entry._BlcScalars[2]);
    _spBLCtrl_Scl11Int->UpdateValue(entry._BlcScalars[3]);

    if (_spAnr)
    {
        _combinedGain = entry._CombinedNoise;
        _darkNoise = entry._DarkNoise;
        _darkFrame = frame_data_t{};
        _sceneFrames.clear();
        _spAnrCombinedGain->UpdateValue(_combinedGain);
        _spAnrDarkNoise->UpdateValue(_darkNoise);
        _spAnr->ApplyLuts(_combinedGain, _darkNoise);
    }

    _spWBController->SetGain(_selectedGain);
    _spWBController->ApplyBlackLevel();
}


void VvpIspProfileBlackLevelControls::ReadBLSAndUpdateUI()
{
    uint32_t cfa[4];
    uint32_t scalars[4];

    _spBlackLevelStats->GetPedestalsAndScalarsForCurrentConfig(&cfa[0], &cfa[1], &cfa[2], &cfa[3],
                                                               &scalars[0], &scalars[1], &scalars[2], &scalars[3]);
    _spProfile->UpdateBlcParametersForGain(_selectedGain, cfa, scalars);

    _spBLCtrl_Ped00Int->UpdateValue(cfa[0]);
    _spBLCtrl_Ped01Int->UpdateValue(cfa[1]);
    _spBLCtrl_Ped10Int->UpdateValue(cfa[2]);
    _spBLCtrl_Ped11Int->UpdateValue(cfa[3]);

    _spBLCtrl_Scl00Int->UpdateValue(scalars[0]);
    _spBLCtrl_Scl01Int->UpdateValue(scalars[1]);
    _spBLCtrl_Scl10Int->UpdateValue(scalars[2]);
    _spBLCtrl_Scl11Int->UpdateValue(scalars[3]);

    _spWBController->SetGain(_selectedGain);
    _spWBController->ApplyBlackLevel();
}

void VvpIspProfileBlackLevelControls::MacroControlReadBlackLevels()
{
    std::vector<float> gains = _spProfile->GetListOfGains();
    uint32_t currentSelectionIndex = _spGainSelectEnum->GetSelectedIndex();

    if (gains.size() == 0)
    {
        return;
    }

    for (int i = 0; i < (int)gains.size(); i++)
    {
        SelectGain(gains[i]);
        usleep(66000); // Wait for 2-4 frames depending on fps FIXME - better delay?
        ReadBLSAndUpdateUI();
    }

    // Now re-select the current item
    _spGainSelectEnum->UpdateValue(currentSelectionIndex, true);
}

void VvpIspProfileBlackLevelControls::UpdateUiWithCurrentProfile()
{
    ResetGainDropdownAndSelectGain(1.0);
}

void VvpIspProfileBlackLevelControls::ResetGainDropdownAndSelectGain(float gain)
{
    int gainSelectLength = _spGainSelectEnum->GetLength(); // Because it gets smaller and smaller in each loop iteration
    int selectedIndex = 0;

    //std::cout << "GainSelect length: " << gainSelectLength << "\n";

    // Update the gain selection list
    // First we need to clear all the items
    for (int i = 0; i < gainSelectLength; i++)
    {
        //std::cout << "Removing index " << 0 << ": " << _spGainSelectEnum->GetOption(0)._label << "\n";
        // 0 is because the list resizes, so we just remove from the top until they're all gone
        _spGainSelectEnum->RemoveEnumItem(0, _spGainSelectEnum->GetOption(0)._label);
    }

    std::vector<float> gains = _spProfile->GetListOfGains();
    //std::cout << "gain length: " << gains.size() << "\n";
    if (gains.size() == 0)
    {
        _spGainSelectEnum->Enable(false);
        _spBLCtrl_Ped00Int->Enable(false);
        _spBLCtrl_Ped01Int->Enable(false);
        _spBLCtrl_Ped10Int->Enable(false);
        _spBLCtrl_Ped11Int->Enable(false);

        _spBLCtrl_Scl00Int->Enable(false);
        _spBLCtrl_Scl01Int->Enable(false);
        _spBLCtrl_Scl10Int->Enable(false);
        _spBLCtrl_Scl11Int->Enable(false);
        _spReadBLSButton->Enable(false);
        _spMacroReadBLSButton->Enable(false);
        return;
    }
    else
    {
        _spGainSelectEnum->Enable(true);
        _spBLCtrl_Ped00Int->Enable(true);
        _spBLCtrl_Ped01Int->Enable(true);
        _spBLCtrl_Ped10Int->Enable(true);
        _spBLCtrl_Ped11Int->Enable(true);

        _spBLCtrl_Scl00Int->Enable(true);
        _spBLCtrl_Scl01Int->Enable(true);
        _spBLCtrl_Scl10Int->Enable(true);
        _spBLCtrl_Scl11Int->Enable(true);
        _spReadBLSButton->Enable(true);

        if (gains.size() > 1)
        {
            _spMacroReadBLSButton->Enable(true);
        }
        else
        {
            _spMacroReadBLSButton->Enable(false);
        }
    }

    for (int i = 0; i < (int)gains.size(); i++)
    {
        if (gains[i] == gain)
        {
            selectedIndex = i;
        }

        _spGainSelectEnum->AddEnumItem(std::to_string(gains[i]), i);
    }

    _spGainSelectEnum->UpdateValue(selectedIndex, true);
}

void VvpIspProfileBlackLevelControls::LoadFunctionPointerForUpdatingOverviewUI(std::function<void(void)> fpUpdateOverviewUiCB)
{
    _fpUpdateOverviewUiCB = std::move(fpUpdateOverviewUiCB);
}