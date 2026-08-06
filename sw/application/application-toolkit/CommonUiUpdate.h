/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "ParamList.h"
#include "AppToolkitRawRgbImage.h"
#include "AppToolkitIJson.h"
#include <functional>
#include "AppToolkitIJson.h"

class ImageClassificationGroup;
class UiParameterItem;
class UiControlItem;
class UiElement;
class UiControlContainer;

using UiJsonParameters = std::unordered_map<std::string, AtUtils::JsonValueVariant>;
using ReceiveDataCB = std::function<void(uint32_t clientId, const std::vector<uint8_t>& data)>;

class UiUpdateCommand
{
public:
    std::string         _name;
    uint32_t            _clientId;
    UiJsonParameters    _parameters;
};

class UiUpdate
{
public:
    // New style commands
    static void LoadImageFile(uint32_t resizeWidth, uint32_t resizeHeight, bool crop, uint32_t loadInPictureID, ReceiveDataCB receiveImageCB, uint32_t clientId);
    static void ShowWaiting(bool show, uint32_t clientId = allClients);
    static void PingUiClients(uint32_t counter);
    static void CallJavaScriptFunction(uint32_t controlID, std::string functionName,
                                       std::string parameter, uint32_t client_id = allClients);
    static void ControlItemAddEnum(UiControlItem* control_item,
                                     const std::string& value);
    static void ShowColorPicker(uint32_t clientId, AppToolkit::RgbPixel colour, std::string label, bool preview);
    static void NewInfoLabel(const char* label);
    static void ControlItemUpdateLabel(UiControlItem* pControlItem, const std::string& label);
    static void ControlItemUpdateEnabled(UiControlItem* pControlItem);
    static void ControlItemUpdateShow(UiControlItem* pControlItem);
    static void ControlContainerUpdateShow(UiControlContainer* pContainer);
    static void ControlItemRemoveEnum(UiControlItem* pControlItem, int index);
    static void ControlItemUpdateRange(UiControlItem* control_item, const std::string& min, const std::string& max);
    static void SendCommand(const UiUpdateCommand& command);

    static void SendLoggerMessage(const std::string& data);

    virtual ~UiUpdate() {}
    static constexpr uint32_t allClients = (uint32_t)-1;
    static constexpr uint32_t invalidClient = (uint32_t)0;
    size_t Notify();

protected:
    UiUpdate(uint32_t clientId = allClients,
             uint32_t excludeClientId = invalidClient)
    :	_clientId(clientId)
    ,	_excludeClientId(excludeClientId)
    {
    }

public:
    virtual bool IsEmpty() { return false; }
    std::shared_ptr<std::string> GetJsonString();
    virtual bool UpdateClient(uint32_t clientId)
    {
        if ((_excludeClientId != invalidClient) && (clientId == _excludeClientId))
            return false;
        else if (_clientId == allClients)
            return true;
        else
            return (clientId == _clientId);
    }

protected:
    std::shared_ptr<std::string> _jsonString;
    uint32_t _clientId;
    uint32_t _excludeClientId;
};

class JsonDOM
{
public:
    JsonDOM(const char* nameValue);
    AtUtils::IJsonObjectPtr GetRoot();
    AtUtils::IJsonPtr GetDOM() { return _spJsonDOM; }
    std::shared_ptr<std::string> ToString();

private:
    AtUtils::IJsonPtr _spJsonDOM;
    AtUtils::IJsonObjectPtr _topLevel;
};

class UiUpdateJSON : public JsonDOM
{
public:
    UiUpdateJSON() : JsonDOM("UiUpdate")
    {
    }
};

using DialogClosedCB = std::function<void(uint32_t clientId, bool OK)>;

class ModalDialogUiUpdate : public UiUpdate
{
public:
    enum class DialogType
    {
        CLOSE,
        PICTURE,
        WARP_FULL_CONTROLS,
        TEXT_VIEW,
        MESSAGE,
        PICTURE_IN_PICTURE,
        COLOUR_PICKER,
        SETTINGS
    };

    ModalDialogUiUpdate(uint32_t clientId,
                        DialogType dialogType,
                        const std::string& message = "",
                        DialogClosedCB okCallback = nullptr,
                        DialogClosedCB cancelCallback = nullptr);

    static void CallCloseCallback(uint32_t clientId, bool okButton);

private:
    static DialogClosedCB _okCallback;
    static DialogClosedCB _cancelCallback;
};

void UiMessage(const std::string& message,
               DialogClosedCB okCallback = nullptr,
               DialogClosedCB cancelCallback = nullptr,
               uint32_t clientId = UiUpdate::allClients);

class ShowTextFileDialogUiUpdate : public UiUpdate
{
public:
    ShowTextFileDialogUiUpdate(uint32_t clientId,
                               std::string title,
                               std::vector<std::string>& stringContents);
};

// class UiLoggerUpdate : public UiUpdate
// {
// public:
//     UiLoggerUpdate(uint32_t clientId, std::string data);
// };

class PiPDetails;

class PiPDialogUiUpdate : public UiUpdate
{
public:
    PiPDialogUiUpdate(uint32_t clientId,
                      std::shared_ptr<PiPDetails> spPiPDetails);
};

void NewInfoLabel(const char* format, ...);

class UiElementValueUpdate : public UiUpdate
{
public:
    UiElementValueUpdate(UiControlItem* pControlItem,
                         const AtUtils::JsonValueVariant& value,
                         bool stringIsObject = false,
                         uint32_t excludeClientId = invalidClient);
};


// class UiElementEnumAddUpdate : public UiUpdate
// {
// public :
//     UiElementEnumAddUpdate(UiControlItem* pControlItem,
//                            std::string value);
// };

class UiControlsUpdate : public UiUpdate
{
public:
    UiControlsUpdate();
};

class ExportFileUiUpdate : public UiUpdate
{
public:
    ExportFileUiUpdate(uint32_t clientId, std::filesystem::path filePath);
};

using ImportCompleteUserActionCB = std::function<void(std::filesystem::path importFile,
                                                      std::filesystem::path importDestination)>;
using ImportCompleteActionCB = std::function<void(std::filesystem::path importFile)>;

class ImportFileUiUpdate : public UiUpdate
{
public:
    ImportFileUiUpdate(uint32_t clientId, std::filesystem::path filePath,
                       std::string dialogTitle,
                       ImportCompleteUserActionCB importCompleteAction = nullptr,
                       bool multipleFiles = false);

    static void NewFileReceived(std::filesystem::path filePath);
    static std::filesystem::path _fileImportDestinationPath;
    static ImportCompleteActionCB _importCompleteAction;
};

class AutoShowWaiting : public UiUpdate
{
public:
    AutoShowWaiting(uint32_t clientId = allClients);
    ~AutoShowWaiting();

private:
    uint32_t _clientId;
};
