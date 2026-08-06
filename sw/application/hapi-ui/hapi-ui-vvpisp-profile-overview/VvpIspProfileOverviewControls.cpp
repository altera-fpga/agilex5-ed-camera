/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspProfileOverviewControls.h"
#include "UiElements.h"

#include <string>

static const char* profileFilename = "awb_profile.json";

VvpIspProfileOverviewControls::VvpIspProfileOverviewControls(const std::shared_ptr<SensorCalibrationProfile>& spProfile)
: _spProfile(spProfile)
{

}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileOverviewControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Profile Overview", GetSettingsSectionName());

    auto importProfileCB = [this](uint32_t clientID) 
    {

        std::filesystem::path fileImportDestination = std::filesystem::current_path();
        fileImportDestination /= profileFilename;

        auto importCompleteAction = [this, clientID](std::filesystem::path tempFilePath,
                                                     std::filesystem::path importDestinationPath) 
        {
            std::error_code ec;
            std::filesystem::copy(tempFilePath, importDestinationPath,
                                  std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::remove(tempFilePath, ec);
#ifdef __linux__
            ::sync();
#endif    
            auto profileTitle = tempFilePath.filename();
            profileTitle.replace_extension();

            auto error_code = _spProfile->LoadFromFile(profileFilename, importDestinationPath);

            if (error_code == ProfileErrors::OK)
            {
                if (_updateAllCalibUiCB)
                {
                    _updateAllCalibUiCB(profileTitle);
                }
            }
            else
            {
                // It didn't load so remove it
                std::error_code ec;
                std::filesystem::remove(importDestinationPath, ec);

                UiMessage("Failed to load profile");
            }
        };
        ImportFileUiUpdate importFile(clientID, std::move(fileImportDestination), "Camera profile", importCompleteAction);
    };

    spContainer->AddButtonControl("Import camera profile", importProfileCB);

    auto sensorInputTextCB = [this](uint32_t clientID, std::string& value) -> void 
    {
        //_spProfile->SetSensor(value);
        //_spCurrentProfileSensorLabel->UpdateValue(value);
    };
    _spSensorTextbox = spContainer->AddTextControl("Sensor:", "[None]", sensorInputTextCB);

    _spProfileVersionLabel = spContainer->AddLabelControl("Version:", "[None]");
    _spProfileValidLabel = spContainer->AddLabelControl("Profile Valid:", "No");
    _spProfileNumAnalogueGainsLabel = spContainer->AddLabelControl("No. Analog Gains:", "0");
    _spProfileNumLumSamplesLabel = spContainer->AddLabelControl("No. Lum Samples:", "0");
    _spProfileBaselineTempLabel = spContainer->AddLabelControl("Baseline Color Temp: (K)", "6500");
    _spProfileNumTempSamplesLabel = spContainer->AddLabelControl("No. Temp Samples:", "0");
    _spProfileANRCoeffsPresentLabel = spContainer->AddLabelControl("ANR Coeffs Present:", "No");
    _spProfileVCMeshPresentLabel = spContainer->AddLabelControl("Vignette Mesh Present:", "No");
    _spProfileVCMeshDimensionLabel = spContainer->AddLabelControl("Vignette Mesh Dimension:", "N/A");

    auto exportProfileCB = [this](uint32_t clientID) 
    {
        ExportProfile(clientID);
    };
    spContainer->AddButtonControl("Export", exportProfileCB);


    return {std::move(spContainer)};
}


void VvpIspProfileOverviewControls::ExportProfile(uint32_t clientId)
{
    std::string filename = _spProfile->GetSensor() + "_" + _spProfile->GetVersion() + ".json";
    _spProfile->SaveToFile(filename);
    std::filesystem::path settingsFilePath = std::filesystem::current_path();
    settingsFilePath /= filename;
    ExportFileUiUpdate exportFile(clientId, std::move(settingsFilePath));
}

void VvpIspProfileOverviewControls::UpdateUiWithCurrentProfile()
{
    if (!_spProfile)
    {
        return;
    }

    _spSensorTextbox->UpdateValue(_spProfile->GetSensor());
    _spProfileVersionLabel->UpdateValue(_spProfile->GetVersion());
    if (_spProfile->IsValid())
    {
        _spProfileValidLabel->UpdateValue("Yes");
    }
    else
    {
        _spProfileValidLabel->UpdateValue("No");
    }
    _spProfileNumAnalogueGainsLabel->UpdateValue(std::to_string(_spProfile->GetListOfGains().size()));
    _spProfileNumLumSamplesLabel->UpdateValue(std::to_string(_spProfile->GetListOfGainScaleReadings().size())); // TODO - this will change when we swap to luminance system
    _spProfileBaselineTempLabel->UpdateValue(std::to_string(_spProfile->GetBaselineColourTemp()));
    _spProfileNumTempSamplesLabel->UpdateValue(std::to_string(_spProfile->GetListOfTemps().size())); // TODO - this will change when we swap to luminance system
    
    _spProfileANRCoeffsPresentLabel->UpdateValue("Yes");
    
    if (_spProfile->HasVCMesh())
    {
        _spProfileVCMeshPresentLabel->UpdateValue("Yes");
    }
    else
    {
        _spProfileVCMeshPresentLabel->UpdateValue("No");
    }
    _spProfileVCMeshDimensionLabel->UpdateValue(std::to_string(_spProfile->GetHorizVCMeshPoints()) + "x" + std::to_string(_spProfile->GetVertiVCMeshPoints()));
}

void VvpIspProfileOverviewControls::AddProfileLoadUiUpdateCallback(std::function<void(std::string)> callback)
{
    _updateAllCalibUiCB = std::move(callback);
}