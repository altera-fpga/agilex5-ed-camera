/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "VvpIspVfwControls.h"
#include "UiElements.h"
#include "Tiff_Encoder.h"


using namespace SwApi;


VvpIspVfwControls::VvpIspVfwControls(std::shared_ptr<IFrameCapture> spIFrameCapture, const std::string& name):
    _spIFrameCapture{std::move(spIFrameCapture)},
    _name(name)
{
}


std::vector<std::shared_ptr<UiControlContainer>> VvpIspVfwControls::AddUiElements() 
{
    if (!_spIFrameCapture) 
    {
        return {};
    }

    auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());


    auto rawSensorSnapshotCB = [this](uint32_t clientID)
    {
        const auto frame = _spIFrameCapture->CaptureRawFrame(16, true);
        GenerateImageFile(frame);
    };

    spContainer->AddButtonControl("Raw Camera Snapshot", rawSensorSnapshotCB);

    auto pipelineSnapshotCB = [this](uint32_t clientID)
    {
        const auto frame = _spIFrameCapture->CaptureProcessedFrame(16, true);
        GenerateImageFile(frame);
    };

    spContainer->AddButtonControl("ISP Output Snapshot", pipelineSnapshotCB);


    auto downloadCB = [this](uint32_t clientID)
    {
        std::string filename = "out.tif";
        std::filesystem::path settingsFilePath = std::filesystem::current_path();
        settingsFilePath /= filename;
        ExportFileUiUpdate exportFile(clientID, std::move(settingsFilePath));
    };
    spContainer->AddButtonControl("Download Image", downloadCB);

    return {std::move(spContainer)};
}


void VvpIspVfwControls::GenerateImageFile(const SwApi::vfw_frame_t& frame)
{    
    Tiff_Encoder tiffWriter;
    Video_Type videoType;

    videoType.init(Video_Type::FourCC::RGB_BI_BITFIELDS, frame._width, frame._height, 0); // Ignore the stride for now

    auto encodingOffers = tiffWriter.get_compatible_encoding_methods(videoType, 16);

    tiffWriter.set_encoding_method(encodingOffers[0].solution, videoType);

    tiffWriter.open_filename("out.tif");

    if (!tiffWriter.bad())
    {
        try
        {
            tiffWriter.write_tiff_header();
            tiffWriter.store_bytes(frame._data.data(), frame._data.size());
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error writing captured image to disk: " << e.what() << '\n';
        }
    }

    bool write_error = tiffWriter.bad();
    tiffWriter.close();

    std::string ui_msg{write_error ? "No space left on disk!" : "Snapshot saved successfully"};

    UiMessage(ui_msg);
}
