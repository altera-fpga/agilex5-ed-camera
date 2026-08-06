/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "LoggerControl.h"
#include "CommonApplicationBase.h"


UiLoggerControl::UiLoggerControl():
    UiControlItem("UiLogger")
{
}

void UiLoggerControl::FillJson(AtUtils::IJsonObjectPtr &spJsonObject)
{
    UiControlItem::FillJson(spJsonObject);

    std::string value;

    for(const auto&s : _data)
    {
        value += s;
        value += "\\n";
    }

    spJsonObject->AddValue("value", value);
}

AtUtils::JsonValueVariant UiLoggerControl::UpdateValue(const std::string& value)
{
    // Reflect the validated value to all clients
    UiElementValueUpdate update_ui(this, value);

    _data.emplace_back(value);

    return value;
}


bool UiLoggerControl::SaveToFile(const std::string& filename)
{
    bool ret = false;
    std::ofstream ofs(filename.c_str());

    if(ofs.is_open())
    {
        for(const auto& s : _data)
        {
            ofs << s << "\n";
        }

        ofs.close();
        ret = true;
    }

    return ret;
}


LoggerControl::LoggerControl()
{}


std::vector<std::shared_ptr<UiControlContainer>> LoggerControl::AddUiElements()
{
    auto panel = std::make_shared<UiControlContainer>("Logger", GetSettingsSectionName());

    _uiControl = std::make_shared<UiLoggerControl>();

    panel->Add(_uiControl, "", "");

    panel->SetLeftAnchor({ ANCHOR_TYPE::PARENT_NEAR, 0 });
    panel->SetRightAnchor({ ANCHOR_TYPE::FIXED_SIZE, 800 });
    panel->SetBottomAnchor({ ANCHOR_TYPE::FIXED_SIZE, 780 });

    auto saveLogCB = [this](uint32_t clientID){
        std::cout << "Save log clicked\n";

        std::filesystem::path logFilename = std::filesystem::current_path();
        static const char* logFileTitle = "wt3-log.txt";

        logFilename /= logFileTitle;

        if (_uiControl->SaveToFile(logFilename))
        {
            ExportFileUiUpdate exportFile(clientID, std::move(logFilename));
        }        
    };

    panel->AddButtonControl("Save log", saveLogCB);

    return { std::move(panel) };
}


void LoggerControl::LogMessage(const std::string& msg)
{
    _uiControl->UpdateValue(msg);
}
