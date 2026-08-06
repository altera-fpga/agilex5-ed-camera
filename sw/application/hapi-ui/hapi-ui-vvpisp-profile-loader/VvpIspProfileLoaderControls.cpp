/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspProfileLoaderControls.h"
#include "UiElements.h"

#include "CoeffGen.h"

#include <string>

static const char* profileFilename = "awb_profile.json";

static const uint32_t panelWidth = 450;
static const uint32_t margin = 12;

VvpIspProfileLoaderControls::VvpIspProfileLoaderControls(const std::string& name,
                                                         const std::shared_ptr<SensorCalibrationProfile>& spProfile,
                                                         const std::shared_ptr<WhiteBalanceController>& spWBController)
: _name(name),
  _spProfile(spProfile),
  _spWBController(spWBController)
{
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspProfileLoaderControls::AddUiElements()
{
    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

    _spCurrentProfileFeedbackLabel = spContainer->AddLabelControl("Current profile", "[None]");

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
            LoadProfile(profileTitle, std::move(importDestinationPath));

        };
        ImportFileUiUpdate importFile(clientID, std::move(fileImportDestination), "Camera profile", importCompleteAction);
    };
    spContainer->AddButtonControl("Import Camera Profile", importProfileCB);

    return {std::move(spContainer)};
}

void VvpIspProfileLoaderControls::LoadProfile(const std::string& profileTitle, const std::filesystem::path& importDestinationPath)
{
    auto error_code = _spProfile->LoadFromFile(profileFilename, importDestinationPath);

    if (error_code == ProfileErrors::OK)
    {
        _spCurrentProfileFeedbackLabel->UpdateValue(profileTitle);
        //_spCurrentProfileVersionLabel->UpdateValue(_spProfile->GetVersion());

        // TODO - put this in a callback in AE
        _spWBController->ApplyBlackLevel();
        _spWBController->ApplyVCMeshes();
        _spWBController->ApplyWhiteBalance();
    }
    else
    {
        _spCurrentProfileFeedbackLabel->UpdateValue(ToString(error_code));

        // It didn't load so remove it
        std::error_code ec;
        std::filesystem::remove(importDestinationPath, ec);

        UiMessage("Failed to load profile");
    }
    //_spCurrentProfileOptionStatusLabel->UpdateValue(ToString(error_code));
}

void VvpIspProfileLoaderControls::ProfileLoadedExternally(const std::string& title)
{
    _spCurrentProfileFeedbackLabel->UpdateValue(title);
    //_spCurrentProfileVersionLabel->UpdateValue(_spProfile->GetVersion());
    //_spCurrentProfileOptionStatusLabel->UpdateValue(ToString(ProfileErrors::OK));

    // TODO - put this in a callback in AE
    _spWBController->ApplyBlackLevel();
    _spWBController->ApplyVCMeshes();
    _spWBController->ApplyWhiteBalance();
}
