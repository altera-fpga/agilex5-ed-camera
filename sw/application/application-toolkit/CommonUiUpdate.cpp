/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "CommonUiUpdate.h"
#include <sstream>
#include "CommonUiLayer.h"
#include "UiElements.h"
#include "CommonApplicationBase.h"
#include "PictureInPictureHandler.h"
#include "DataTransferWebSocketCommandProcessor.h"

#ifdef __linux__
#include <unistd.h>
#endif

size_t UiUpdate::Notify()
{
    size_t nObservers = 0;
    ICommonApplicationBase* pAppCore = CommonApplicationBase::Get();
    if (pAppCore)
        nObservers = pAppCore->GetUiNotifier()->Notify(*this);

    return nObservers;
}

std::shared_ptr<std::string> UiUpdate::GetJsonString()
{
    return _jsonString;
}

JsonDOM::JsonDOM(const char* nameValue)
{
    _spJsonDOM = AtUtils::IJson::Create();
    _topLevel = _spJsonDOM->RootObject()->AddObject(nameValue);
}

AtUtils::IJsonObjectPtr JsonDOM::GetRoot()
{
    return _topLevel;
}

std::shared_ptr<std::string> JsonDOM::ToString()
{
    return std::make_shared<std::string>(_spJsonDOM->ToString());
}

///////////////////////////////////////////////////////////////////////////////

DialogClosedCB ModalDialogUiUpdate::_okCallback;
DialogClosedCB ModalDialogUiUpdate::_cancelCallback;

ModalDialogUiUpdate::ModalDialogUiUpdate(uint32_t clientId,
                                         DialogType dialogType,
                                         const std::string& message,
                                         DialogClosedCB okCallback,
                                         DialogClosedCB cancelCallback)
:	UiUpdate(clientId)
{
    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandObject = node->AddObject("ModalDialog");
    std::string dialogTypeString;
    std::string button0String(okCallback ? "OK" : "");
    std::string button1String(cancelCallback ? "Cancel" : "");
    _okCallback = std::move(okCallback);
    _cancelCallback = cancelCallback;

    if (dialogType == DialogType::CLOSE)
        dialogTypeString = "CloseDialogs";
    else if (dialogType == DialogType::PICTURE)
        dialogTypeString = "PictureDialog";
    else if (dialogType == DialogType::SETTINGS)
        dialogTypeString = "SettingsDialog";
    else if (dialogType == DialogType::WARP_FULL_CONTROLS)
        dialogTypeString = "WarpControls";
    else if (dialogType == DialogType::TEXT_VIEW)
        dialogTypeString = "OJTextViewDialog";
    else if (dialogType == DialogType::MESSAGE)
    {
        dialogTypeString = "MessageDialog";
        if (!cancelCallback)
            button0String = "Close";
    }
    else if (dialogType == DialogType::PICTURE_IN_PICTURE)
        dialogTypeString = "PictureInPicture";
    else if (dialogType == DialogType::COLOUR_PICKER)
        dialogTypeString = "ColourPicker";

    spCommandObject->AddValue("dialog_type", dialogTypeString);
    spCommandObject->AddValue("message", message);
    spCommandObject->AddValue("button_0", button0String.c_str());
    spCommandObject->AddValue("button_1", button1String.c_str());
    _jsonString = jsonDOM.ToString();
}

void ModalDialogUiUpdate::CallCloseCallback(uint32_t clientId, bool okButton)
{
    DialogClosedCB dialogClosedCB = okButton ? _okCallback : _cancelCallback;
    if (dialogClosedCB)
        dialogClosedCB(clientId, okButton);

    _okCallback = nullptr;
    _cancelCallback = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

ShowTextFileDialogUiUpdate::ShowTextFileDialogUiUpdate(uint32_t clientId,
                                                       std::string title,
                                                       std::vector<std::string>& stringContents)
:	UiUpdate(clientId)
{
    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr spNode = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandNode = spNode->AddObject("ModalDialog");
    spCommandNode->AddValue("dialog_type", "TextViewDialog");
    spCommandNode->AddValue("title", title);

    auto spLinesArray = spCommandNode->AddArray("Lines");

    for (auto line : stringContents)
    {
        auto spLineObject = spLinesArray->AddElement()->AddObject();
        AtUtils::MakeSafeXml(line);
        spLineObject->AddValue("text", line.c_str());
    }
    _jsonString = jsonDOM.ToString();
}

///////////////////////////////////////////////////////////////////////////////


// UiLoggerUpdate::UiLoggerUpdate(uint32_t clientId, std::string data):
//     UiUpdate(clientId)
// {
// SendCommand({ "UiElementLabelUpdate", allClients, { { "id", pControlItem->GetID() }, { "label", label } } });
// }

///////////////////////////////////////////////////////////////////////////////

PiPDialogUiUpdate::PiPDialogUiUpdate(uint32_t clientId,
                                     std::shared_ptr<PiPDetails> spPiPDetails)
:	UiUpdate(clientId)
{
    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandObject = node->AddObject("ModalDialog");
    spCommandObject->AddValue("dialog_type", "PictureInPicture");
    spCommandObject->AddValue("output_width", spPiPDetails->_outputWidth);
    spCommandObject->AddValue("output_height", spPiPDetails->_outputHeight);

    auto spLayersArray = spCommandObject->AddArray("Layers");

    for (auto& layer : spPiPDetails->_layers)
    {
        auto spLayerObject = spLayersArray->AddElement()->AddObject();

        spLayerObject->AddValue("index", layer._index);
        spLayerObject->AddValue("name", layer._name);
        spLayerObject->AddValue("x", layer._x);
        spLayerObject->AddValue("y", layer._y);
        spLayerObject->AddValue("width", layer._width);
        spLayerObject->AddValue("height", layer._height);
        spLayerObject->AddValue("minimum_x", layer._minimumX);
        spLayerObject->AddValue("minimum_y", layer._minimumY);
        spLayerObject->AddValue("maximum_x", layer._maximumX);
        spLayerObject->AddValue("maximum_y", layer._maximumY);
        spLayerObject->AddValue("minimum_width", layer._minimumWidth);
        spLayerObject->AddValue("minimum_height", layer._minimumHeight);
        spLayerObject->AddValue("maximum_width", layer._maximumWidth);
        spLayerObject->AddValue("maximum_height", layer._maximumHeight);
        spLayerObject->AddValue("z_index", layer._zIndex);
        spLayerObject->AddValue("alpha", layer._alpha);
        spLayerObject->AddValue("colour", layer._colour);
        spLayerObject->AddValue("fixed_size", layer._fixedSize);
        spLayerObject->AddValue("fixed_aspect_ratio", layer._fixedAspectRatio);
    }

    _jsonString = jsonDOM.ToString();
}


void UiUpdate::ShowColorPicker(uint32_t clientId, AppToolkit::RgbPixel colour,
                               std::string label, bool preview)
{
    UiUpdateCommand command =
    {
        "ModalDialog", clientId,
        {
            { "dialog_type", "ColourPicker" },
            { "label", label },
            { "red", colour._red },
            { "green", colour._green },
            { "blue", colour._blue },
            { "preview", preview }
        }
    };

    SendCommand(command);
}

void UiUpdate::NewInfoLabel(const char* label)
{
    SendCommand({ "NewInfoLabel", allClients, { { "label", label } } });
}

void NewInfoLabel(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int formatted_length = AtUtils::vscprintf(format, args);
    char* buffer = new char[formatted_length + 1];
#ifdef linux
    vsprintf(buffer, format, args);
#else
    vsprintf_s(buffer, formatted_length + 1, format, args);
#endif
    UiUpdate::NewInfoLabel(buffer);

    va_end(args);
    delete [] buffer;
}

///////////////////////////////////////////////////////////////////////////////

UiElementValueUpdate::UiElementValueUpdate(UiControlItem* pControlItem,
                                           const AtUtils::JsonValueVariant& value,
                                           bool stringIsObject,
                                           uint32_t excludeClientId)
:	UiUpdate(allClients, excludeClientId)
{
    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandObject = node->AddObject("UiElementValueUpdate");
    spCommandObject->AddValue("id", pControlItem->GetID());
    auto spJsonValue = spCommandObject->AddValue("value", value);
    if (stringIsObject)
        spJsonValue->SetStringIsObject();

    _jsonString = jsonDOM.ToString();
    Notify();
}

void UiUpdate::ControlItemUpdateLabel(UiControlItem* pControlItem, const std::string& label)
{
    SendCommand({ "UiElementLabelUpdate", allClients, { { "id", pControlItem->GetID() }, { "label", label } } });
}

void UiUpdate::ControlItemUpdateEnabled(UiControlItem* pControlItem)
{
    SendCommand({ "UiElementEnabledUpdate", allClients, { { "id", pControlItem->GetID() }, { "enabled", pControlItem->GetEnabled() } } });
}

void UiUpdate::ControlItemUpdateShow(UiControlItem* pControlItem)
{
    SendCommand({ "UiElementShow", allClients, { { "id", pControlItem->GetID() }, { "show", pControlItem->GetShown() } } });
}

void UiUpdate::ControlContainerUpdateShow(UiControlContainer* pContainer)
{
    SendCommand({ "UiControlContainerShow", allClients, { { "id", pContainer->GetID() }, { "show", pContainer->GetShown() } } });
}

// void UiUpdate::SendLoggerMessage(const std::string& data)
// {
//     SendCommand({ "UiLoggerMessage", allClients, { { "data",  data } } });
// }

///////////////////////////////////////////////////////////////////////////////

UiControlsUpdate::UiControlsUpdate()
{
    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandObject = node->AddObject("UiControlsUpdate");
    CommonUiLayer::Get()->GetUiControls(spCommandObject);
    _jsonString = jsonDOM.ToString();
}

void UiUpdate::ControlItemRemoveEnum(UiControlItem* pControlItem, int index)
{
    SendCommand({ "UiControlEnumValueRemove", allClients, { { "id", pControlItem->GetID() }, { "value", index } } });
}

void UiUpdate::ControlItemUpdateRange(UiControlItem* control_item, const std::string& min, const std::string& max)
{
    SendCommand({ "UiControlUpdateRange", allClients, { { "id", control_item->GetID() }, { "min", min }, { "max", max } } });
}

///////////////////////////////////////////////////////////////////////////////

ExportFileUiUpdate::ExportFileUiUpdate(uint32_t clientId, std::filesystem::path filePath)
:	UiUpdate(clientId)
{
    bool file_copy_ok = false;

    try
    {
        std::filesystem::create_directories("WebToolkit/export-files");
        std::filesystem::copy(filePath, "WebToolkit/export-files/last-export.file",
                            std::filesystem::copy_options::overwrite_existing);
        file_copy_ok = true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error exporting file: " << e.what() << '\n';
    }

    if(file_copy_ok)
    {

#ifdef __linux__
            ::sync();
#endif

        UiUpdateJSON jsonDOM;
        AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
        AtUtils::IJsonObjectPtr spCommandObject = node->AddObject("ExportFile");
        spCommandObject->AddValue("url", "export-files/last-export.file");
        spCommandObject->AddValue("file_name", filePath.filename().string());
        _jsonString = jsonDOM.ToString();
        Notify();                            
    }
    else
    {
        UiMessage("No space left of disk!", nullptr, nullptr, clientId);
    }
}

ImportCompleteActionCB ImportFileUiUpdate::_importCompleteAction;
std::filesystem::path ImportFileUiUpdate::_fileImportDestinationPath;

ImportFileUiUpdate::ImportFileUiUpdate(uint32_t clientId,
                                       std::filesystem::path importDestinationPath,
                                       std::string dialogTitle,
                                       ImportCompleteUserActionCB importCompleteActionCB,
                                       bool multipleFiles)
:	UiUpdate(clientId)
{
    // Set the static function to callback asynchronously after the import
    // has completed
    _fileImportDestinationPath = importDestinationPath;
    _importCompleteAction = [cb = std::move(importCompleteActionCB),
                             importDestinationPath](std::filesystem::path importPath)
    {
        cb(std::move(importPath), importDestinationPath);
    };
    std::string ext = importDestinationPath.extension().string();
    if (ext == ".image") {
        ext = "image/*";
    }

    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandObject = node->AddObject("ImportFile");
    spCommandObject->AddValue("open_dialog_title", dialogTitle);
    spCommandObject->AddValue("destination", importDestinationPath.string().c_str());
    spCommandObject->AddValue("extension", ext);
    spCommandObject->AddValue("multiple_files", multipleFiles);

    _jsonString = jsonDOM.ToString();
    Notify();
}

void ImportFileUiUpdate::NewFileReceived(std::filesystem::path tempFilePath)
{
    if (ImportFileUiUpdate::_importCompleteAction)
    {
        ImportFileUiUpdate::_importCompleteAction(std::move(tempFilePath));
    }
}

///////////////////////////////////////////////////////////////////////////////

void UiMessage(const std::string& message,
               DialogClosedCB okCallback,
               DialogClosedCB cancelCallback,
               uint32_t clientId)
{
    ModalDialogUiUpdate uiMessage(clientId,
                                  message.empty() ? ModalDialogUiUpdate::DialogType::CLOSE : ModalDialogUiUpdate::DialogType::MESSAGE,
                                  message,
                                  std::move(okCallback),
                                  std::move(cancelCallback));
    uiMessage.Notify();
}

void UiUpdate::SendCommand(const UiUpdateCommand& command)
{
    UiUpdateJSON jsonDOM;
    AtUtils::IJsonObjectPtr node = jsonDOM.GetRoot();
    AtUtils::IJsonObjectPtr spCommandObject = node->AddObject(command._name.c_str());
    for (auto& parameter : command._parameters)
        spCommandObject->AddValue(parameter.first.c_str(), parameter.second);

    UiUpdate uiUpdate(command._clientId);
    uiUpdate._jsonString = jsonDOM.ToString();
    uiUpdate.Notify();
}

void UiUpdate::LoadImageFile(uint32_t resizeWidth,
                             uint32_t resizeHeight,
                             bool crop,
                             uint32_t loadInPictureID,
                             ReceiveDataCB receiveDataCB,
                             uint32_t clientId)
{
    UiUpdateCommand command =
    {
        "LoadImageFile", clientId,
        {
            { "resize_width", resizeWidth },
            { "resize_height", resizeHeight },
            { "crop", crop },
            { "picture_control_id", loadInPictureID }
        }
    };

    DataTransferWebSocketCommandProcessor::SetReceiver(std::move(receiveDataCB));
    SendCommand(command);
}

void UiUpdate::ShowWaiting(bool show, uint32_t clientId)
{
    SendCommand({ "ShowWaiting", clientId, { { "show", show } } });
}

void UiUpdate::PingUiClients(uint32_t counter)
{
    SendCommand({ "Ping", allClients, { { "counter", counter } } });
}

void UiUpdate::CallJavaScriptFunction(uint32_t controlID,
                                      std::string functionName,
                                      std::string parameter,
                                      uint32_t clientId)
{
    UiUpdateCommand command =
    {
        "CallFunctionOnControl", clientId,
        {
            { "control_id", controlID },
            { "function_name", functionName },
            { "parameter", parameter }
        }
    };

    SendCommand(command);
}

void UiUpdate::ControlItemAddEnum(UiControlItem* control_item,
                                    const std::string& value)
{
    SendCommand({ "UiControlEnumValueAdd", allClients, { { "id", control_item->GetID() }, { "value", value } } });
}


///////////////////////////////////////////////////////////////////////////////

AutoShowWaiting::AutoShowWaiting(uint32_t clientId)
:   _clientId(clientId)
{
    UiUpdate::ShowWaiting(true, _clientId);
}

AutoShowWaiting::~AutoShowWaiting()
{
    UiUpdate::ShowWaiting(false, _clientId);
}


