/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <utility>
#include <sstream>
#include "VvpIspFrameReaderControls.h"
#include "CommonUiUpdate.h"
#include "UiElements.h"



namespace
{

std::string SourceTypeToString(const SwApi::VfrSourceMetadata::SourceType sourceType)
{
    switch(sourceType)
    {
        case SwApi::VfrSourceMetadata::SourceType::Bayer:
            return "Bayer";
        case SwApi::VfrSourceMetadata::SourceType::RGB:
            return "RGB";
        case SwApi::VfrSourceMetadata::SourceType::Unknown:
        default:
            return "-";
    }
}

} // namespace


VvpIspFrameReaderControls::VvpIspFrameReaderControls(std::shared_ptr<SwApi::IspVfrInput> spVfrInput):
    _spVfrInput{std::move(spVfrInput)}
{
}

std::vector<std::shared_ptr<UiControlContainer>> VvpIspFrameReaderControls::AddUiElements()
{
    auto spContainer = std::make_shared<UiControlContainer>("Frame Reader", GetSettingsSectionName());

    _spCurrentFileLabel = spContainer->AddLabelControl("Current File:", "No file loaded");
    _spResolutionLabel = spContainer->AddLabelControl("Resolution:", "-");
    _spBitsPerPixelLabel = spContainer->AddLabelControl("Bits Per Sample:", "-");
    _spSourceTypeLabel = spContainer->AddLabelControl("Source Type:", "-");

    auto importImageCB = [this](uint32_t clientID) {
        // This will be used by the browser image picker dialog to limit the file extensions
        // .image maps to all image types
        std::filesystem::path import_file_pattern = "imported_file.image";

        auto importCompleteAction = [this](std::filesystem::path tempFilePath,
                                           std::filesystem::path import_file_pattern) {
            std::error_code ec;
            
            std::filesystem::path fileImportDestination = std::filesystem::current_path()/"vfr-input";

            if(not std::filesystem::exists(fileImportDestination))
                std::filesystem::create_directories(fileImportDestination, ec);

            // Extract the temporary directory and file extension
            std::filesystem::path tempDir = tempFilePath.parent_path();
            std::filesystem::path fileExtension = tempFilePath.extension();

            std::vector<std::filesystem::path> tempFilesToImport;

            // Iterate over all files with the same extension in the temporary directory
            for (const auto& entry : std::filesystem::directory_iterator(tempDir, ec)) {
                if (entry.is_regular_file() && entry.path().extension() == fileExtension) {
                    std::filesystem::path destinationPath = fileImportDestination / entry.path().filename();
                    
                    std::filesystem::copy(entry.path(), destinationPath,
                                          std::filesystem::copy_options::overwrite_existing, ec);
                    std::filesystem::remove(entry.path(), ec);
                    tempFilesToImport.emplace_back(std::move(destinationPath));
                }
            }

#if defined(__linux__)
            ::sync();
#endif

            if(_spVfrInput)
                _spVfrInput->LoadSequence(std::move(tempFilesToImport));            
        };

        static constexpr bool MULTIPLE_FILES = true;

        ImportFileUiUpdate importFile(clientID,
            std::move(import_file_pattern),
            "Select images",
            importCompleteAction,
            MULTIPLE_FILES);
    };

    spContainer->AddButtonControl("Import Images", importImageCB);

    static constexpr int32_t FRI_FR_MIN = 4;
    static constexpr int32_t FRI_FR_MAX = 60;
    static constexpr int32_t FRI_FR_DEFAULT = 60;

    auto frameRateCb = [this](uint32_t clientID, int32_t& v) {
        if(_spVfrInput)
            _spVfrInput->SetFrameRate(std::clamp(v, FRI_FR_MIN, FRI_FR_MAX));
    };

    spContainer->AddSliderControl("Frame Rate", FRI_FR_MIN, FRI_FR_MAX, frameRateCb, "VfrInputFrameRate", FRI_FR_DEFAULT);

    return {std::move(spContainer)};
}


void VvpIspFrameReaderControls::UpdateSourceMetadata(const SwApi::VfrSourceMetadata& metadata)
{
    if(!_spCurrentFileLabel || !_spResolutionLabel || !_spBitsPerPixelLabel || !_spSourceTypeLabel)
    {
        return;
    }

    if(!metadata.IsValid())
    {
        _spCurrentFileLabel->UpdateValue("No file loaded");
        _spResolutionLabel->UpdateValue("-");
        _spBitsPerPixelLabel->UpdateValue("-");
        _spSourceTypeLabel->UpdateValue("-");
        return;
    }

    std::stringstream resolutionLabel;
    resolutionLabel << metadata._width << " x " << metadata._height;

    _spCurrentFileLabel->UpdateValue(metadata._filePath);
    _spResolutionLabel->UpdateValue(resolutionLabel.str());
    _spBitsPerPixelLabel->UpdateValue(std::to_string(metadata._bitsPerSample));
    _spSourceTypeLabel->UpdateValue(SourceTypeToString(metadata._sourceType));
}