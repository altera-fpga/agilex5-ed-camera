/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Lut3dControls.h"

#ifdef __linux__
#include <unistd.h>
#endif


static const char* notLoadedString = "[not loaded]";


Lut3dControls::Lut3dControls(const std::shared_ptr<SwApi::ILut3d>& spLut3d, const std::string& title, const std::string& settingName) :
    _spLut3d(spLut3d),
    _title{title},
    _settingName{settingName}
{
    if (_spLut3d)
    {
        _spLutNameSetting = CreateSettingValue<std::string>("LutName", "");
        std::string lutName = _spLutNameSetting->Get();
        const auto cubeFilePath = GetCubeFilePath();
        _spLut3d->LoadLutFromCubeFile(lutName, cubeFilePath);
    }
}


std::filesystem::path Lut3dControls::GetCubeFilePath()
{
    std::filesystem::path cubeFilePath = std::filesystem::current_path();
    cubeFilePath /= _settingName + ".cube";
    return cubeFilePath;
}


std::vector<std::shared_ptr<UiControlContainer>> Lut3dControls::AddUiElements()
{
    if (!_spLut3d)
        return {};

    auto spContainer = std::make_shared<UiControlContainer>(_title, GetSettingsSectionName());

    uint32_t numValidBuffers = _spLut3d->GetNumValidBuffers();

    auto bypassCB = [this](uint32_t clientID, bool& value) { _spLut3d->SetEnable(!value); };
    _enableCheckbox = spContainer->AddBoolControl("Bypass", bypassCB, "Bypass3DLUT", true);
    if (numValidBuffers == 0)
        _enableCheckbox->Enable(false);

    auto importCubeFileCB = [this](uint32_t clientID) 
    {
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
            auto cubeFileTitle = tempFilePath.filename();
            cubeFileTitle.replace_extension();

            bool enableEnableCheckbox = (_spLut3d->GetNumValidBuffers() == 0);

            if (_spLut3d->LoadLutFromCubeFile(cubeFileTitle, importDestinationPath))
            {
                // Update setting manually
                _spLutNameSetting->Set(cubeFileTitle);

                uint8_t currentLutIndex = _spLut3d->GetActiveBuffer();
                if (currentLutIndex < 2)
                {
                    auto spButton = _spSelectLutButtons[currentLutIndex];
                    if (!spButton->GetEnabled())
                        spButton->Enable(true);

                    spButton->RenameButton(_spLut3d->GetLutName(currentLutIndex));
                }
                UpdateActiveLutButtonLabel();

                if (enableEnableCheckbox)
                    _enableCheckbox->Enable(true);
            }
            else
            {
                // It didn't load so remote it
                std::error_code ec;
                std::filesystem::remove(importDestinationPath, ec);
                _spLutNameSetting->Set("");

                UiMessage("Failed to load cube file");
            }
        };

        const auto cubeFilePath = GetCubeFilePath();

        ImportFileUiUpdate importFile(clientID, std::move(cubeFilePath), "Cube file", importCompleteAction);
    };
    spContainer->AddButtonControl("Import cube file", importCubeFileCB);

    std::string lut0Name = _spLut3d->GetLutName(0);
    std::string lut1Name = _spLut3d->GetLutName(1);
    if (lut0Name.empty())
        lut0Name = notLoadedString;
    if (lut1Name.empty())
        lut1Name = notLoadedString;

    auto selectLut1CB = [this](uint32_t clientID) 
    {
        _spLut3d->SetActiveBuffer(0);
        UpdateActiveLutButtonLabel();
    };
    _spSelectLutButtons[0] = spContainer->AddButtonControl("LUT 1", lut0Name, selectLut1CB);
    if (numValidBuffers < 1)
        _spSelectLutButtons[0]->Enable(false);

    auto selectLut2CB = [this](uint32_t clientID) 
    {
        _spLut3d->SetActiveBuffer(1);
        UpdateActiveLutButtonLabel();
    };
    _spSelectLutButtons[1] = spContainer->AddButtonControl("LUT 2", lut1Name, selectLut2CB);
    if (numValidBuffers < 2)
        _spSelectLutButtons[1]->Enable(false);

    UpdateActiveLutButtonLabel();

    return {std::move(spContainer)};
}

void Lut3dControls::UpdateActiveLutButtonLabel()
{
    uint8_t currentLutIndex = _spLut3d->GetActiveBuffer();
    if (currentLutIndex < 2)
    {
        auto spButton = _spSelectLutButtons[0];
        if (spButton)
            spButton->UpdateLabel((currentLutIndex == 0) ? "<b>LUT 1</b>" : "LUT 1");

        spButton = _spSelectLutButtons[1];
        if (spButton)
        {
            bool enable = ((_spLut3d->GetNumValidBuffers() > 1) && (currentLutIndex == 1));
            spButton->UpdateLabel(enable ? "<b>LUT 2</b>" : "LUT 2");
        }

    }
}