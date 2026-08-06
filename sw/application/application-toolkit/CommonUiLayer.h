/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "ICommand.h"
#include "CommonTypes.h"
#include "Settings.h"
#include "AppToolkitIJson.h"
#include <functional>

class SString;
class CommonUiLayer;
class UiControlContainer;
class UiElement;

extern uint32_t GetNextUniqueId();

class InfoLabel
{
public:
    InfoLabel(const char* label)
    :	_label(label)
    {
        // Remove \n
        AtUtils::Replace(_label, "\n", "");
    }

    void GetJSON(AtUtils::IJsonArrayPtr& spJsonArray)
    {
        spJsonArray->AddElement()->AddObject()->AddValue("label", _label);
    }

    std::string	_label;
};

class UiTab
{
public:
    UiTab(uint32_t tabIndex, 
          const std::string& title,
          const std::string& tooltip) 
    :   _index(tabIndex)
    ,   _title(title)
    ,   _tooltip(tooltip)
    {
    }

    std::vector<std::shared_ptr<UiControlContainer>> _controlContainers;
    uint32_t _index;
    std::string _title;
    std::string _tooltip;
};

class CommonUiLayer
{
public:
    using ConnectionCB = std::function<void(uint32_t clientID)>;

    friend class UiControlsUpdate;
    CommonUiLayer(ConnectionCB newConnectionCB);
    ~CommonUiLayer();

    CommonUiLayer(const CommonUiLayer& other) = delete;
    CommonUiLayer& operator=(const CommonUiLayer& other) = delete;
    CommonUiLayer(CommonUiLayer&& other) = delete;
    CommonUiLayer& operator=(CommonUiLayer&& other) = delete;

    static CommonUiLayer* Get() { return _pInstance; }

    bool Startup(StartupPhase phase);
    bool Shutdown(ShutdownPhase phase);

    std::shared_ptr<std::string> GetMainUserInterfaceXml(uint32_t clientId);
    void RefreshUiControls();
    void LoadSettings(bool restoreDefaults);
    void GetUiControlsFlat(AtUtils::IJsonObjectPtr& spUiJsonObject);
    std::shared_ptr<UiElement> GetControl(const std::string& panelName, const std::string& controlLabel);
    std::shared_ptr<UiElement> GetButton(const std::string& panelName, const std::string& buttonLabel);
    std::shared_ptr<UiElement> GetControl(uint32_t controlID);

protected:
    void AddInfoLabel(const char* string);

private:
    void AddInfoLabels();
    void CreatetUiElements();
    void GetUiControls(AtUtils::IJsonObjectPtr& spUiNode);
    void GetInfoLabels(AtUtils::IJsonObjectPtr& spUiNode);
    void AddUiSettings(AtUtils::IJsonObjectPtr& spUiNode);
    virtual void AddCommands();

    static CommonUiLayer* _pInstance;

    std::vector<std::shared_ptr<InfoLabel>>				_infoLabels;
    std::vector<std::shared_ptr<UiTab>>	                _tabs;
    std::vector<std::shared_ptr<UiControlContainer>>	_dialogControlContainers;
    ConnectionCB _newConnectionCB;
    std::shared_ptr<UiControlContainer> _spSettingsContainer;
};
