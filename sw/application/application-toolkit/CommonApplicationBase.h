/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "IFileImportDestination.h"
#include "ObserverPattern.h"
#include "AtUtils.h"
#include "IpUiControls.h"
#include "CommonTypes.h"
#include "CommonCommands.h"
#include "CommandLine.h"
#include "AppToolkitRawRgbImage.h"
#include "DialogFactory.h"
#include <chrono>
#include "UiConfig.h"
#include "IWebAPI.h"
#include "LoggerControl.h"

class ICommandMap;
class UiUpdate;
class WebServer;
class CommonUiLayer;
class IpUiControls;
class Settings;

using UiNotifier = INotifier<UiUpdate>;
using ImageNotifier = INotifier<std::shared_ptr<AppToolkit::RawRgbImage16>>;

class TopLevelUiTab
{
public:
    TopLevelUiTab(const std::string& title, const std::string& tooltip,
                  BooleanControlCB enabledCB)
    :   _title(title)
    ,   _tooltip(tooltip)
    ,   _enabledCB(std::move(enabledCB))
    ,   _tabIndex(_nextTabIndex++)
    {
    }

    // Constructs a control type, use similar to std::make_shared<>
    template <class CONTROL_TYPE, typename... ARGS>
    std::shared_ptr<CONTROL_TYPE> AddHapiControl(ARGS... args)
    {
        CONTROL_TYPE* newControl = new CONTROL_TYPE(std::move(args)...);
        std::shared_ptr<CONTROL_TYPE> spControlType(newControl);
        std::shared_ptr<IpUiControls> spIpUiControls = std::dynamic_pointer_cast<IpUiControls>(spControlType);
        _allControls.push_back(spIpUiControls);
        return spControlType;
    }

    void AddColumnBreak()
    {
        auto spEmptyControls = AddHapiControl<ColumnBreakPanel>();
        _allControls.push_back(spEmptyControls);
        _spColumnBreaks.push_back(std::move(spEmptyControls));
    }

    std::vector<std::weak_ptr<IpUiControls>> GetAllControls() { return _allControls; }
    void CallCallback(uint32_t clientID, bool enable)
    {
        if (_enabledCB)
            _enabledCB(clientID, enable);
    }

    std::string _title;
    std::string _tooltip;
    BooleanControlCB _enabledCB;
    uint32_t _tabIndex = 0;

private:
    // These are all the IP controls created by the derived class
    std::vector<std::weak_ptr<IpUiControls>> _allControls;
    std::vector<std::shared_ptr<ColumnBreakPanel>> _spColumnBreaks;
    static uint32_t _nextTabIndex;
};

class ICommonApplicationBase : public IFileImportDestination
{
public:
    virtual ~ICommonApplicationBase() {}
    virtual void AddCommand(AtUtils::Message* pCommand, uint32_t deltaTime = 0) = 0;
    virtual void AddCommand(AtUtils::CommandQueueCB commandCB, uint32_t deltaTime = 0) = 0;
    virtual void AddCommand(AtUtils::AutoRepeatCommandQueueCB commandCB, uint32_t deltaTime) = 0;
    virtual void AddCommand(AtUtils::VariableRepeatCommandQueueCB commandCB, uint32_t deltaTime) = 0;
    virtual int Run(bool waitAndShutdown = true) = 0;
    virtual std::vector<std::string> GetLogMessages() = 0;
    virtual ICommandMap* GetCommandMap() = 0;
    virtual AtUtils::MessageQueue* GetMessageQueue() = 0;
    virtual std::vector<std::weak_ptr<TopLevelUiTab>> GetTopLevelTabs() = 0;
    virtual std::shared_ptr<IDialogFactory> GetDialogFactory() = 0;
    virtual void StartShutdown() = 0;
    virtual std::string GetProductName() = 0;
    virtual std::string GetSoftwareVersion() = 0;
    virtual void RefreshUiControls() = 0;
    virtual std::shared_ptr<UiNotifier> GetUiNotifier() = 0;
    virtual std::shared_ptr<ImageNotifier> GetImageNotifier() = 0;
    virtual std::shared_ptr<INotifier<int>> GetActivityNotifier() = 0;
    virtual std::shared_ptr<ISettings> GetSettings() = 0;
    virtual void RestoreFromBackup() = 0;
    virtual void RestoreFactorySettings() = 0;
    virtual void LoadNewSettings(std::shared_ptr<ISettings> spSettings) = 0;
    virtual bool Startup(StartupPhase phase) = 0;
    virtual bool Shutdown(ShutdownPhase phase) = 0;
    virtual void UiConnected(uint32_t clientID) = 0;
    virtual void UiDisconnected(uint32_t clientID) = 0;
    virtual bool IsDebugging() = 0;
    virtual uint32_t GetWindowWidth() = 0;
    virtual uint32_t GetWindowHeight() = 0;
    virtual UiConfig& GetUiConfig() = 0;
};


// Derive you applications main object (eg CoreProcessor) from CommonApplicationBase
class CommonApplicationBase : public ICommonApplicationBase, public IWebAPI
{
public:
    friend class TopLevelUiTab;

    CommonApplicationBase(int argumentCount, char* argumentArray[],
                          const char* productName,
                          const char* softwareVersion,
                          int startupDelaySeconds = 0);
    virtual ~CommonApplicationBase();
    static ICommonApplicationBase* Get();
    static void SetInstance(ICommonApplicationBase* pInstance); // For testing
    void AddCommand(AtUtils::Message* pCommand, uint32_t deltaTime = 0) override;
    void AddCommand(AtUtils::CommandQueueCB commandCB, uint32_t deltaTime = 0) override;
    void AddCommand(AtUtils::AutoRepeatCommandQueueCB commandCB, uint32_t deltaTime) override;
    void AddCommand(AtUtils::VariableRepeatCommandQueueCB commandCB, uint32_t deltaTime) override;

    // Derived application must call Run once it has been constructed
    int Run(bool waitAndShutdown = true) override;

    std::vector<std::string> GetLogMessages() override { return {}; }
    ICommandMap* GetCommandMap() override;
    AtUtils::MessageQueue* GetMessageQueue() override;
    std::vector<std::weak_ptr<TopLevelUiTab>> GetTopLevelTabs() override;
    std::shared_ptr<IDialogFactory> GetDialogFactory() override;
    void StartShutdown() override;
    void ShutdownApp();
    std::string GetProductName() override { return _productName; }
    std::string GetSoftwareVersion() override { return _softwareVersion; }
    void RefreshUiControls() override;
    std::shared_ptr<UiNotifier> GetUiNotifier() override;
    std::shared_ptr<ImageNotifier> GetImageNotifier() override;
    std::shared_ptr<INotifier<int>> GetActivityNotifier() override;
    std::shared_ptr<ISettings> GetSettings() override { return _spSettings; }
    int NewFileImportStarted(std::string& filename, size_t index,
                             size_t numFiles, bool isSequence, uint32_t clientID) override;
    void ImportFile(std::shared_ptr<WebSocketFileTransfer> spWsFileTransfer) override;
    void RestoreFromBackup() override;
    void RestoreFactorySettings() override;
    void LoadNewSettings(std::shared_ptr<ISettings> spSettings) override;
    void UiConnected(uint32_t clientID) override {}
    void UiDisconnected(uint32_t clientID) override {}
    bool IsDebugging() override { return _debugging; }
    uint32_t GetWindowWidth() override { return _windowWidth; }
    uint32_t GetWindowHeight() override { return _windowHeight; }
    UiConfig& GetUiConfig() override { return _uiConfig; }

    // IWebAPI
    std::string GetControlsJSON() override;
    std::string GetControlJSON(uint32_t controlID) override;
    uint32_t GetControlID(const std::string& panelName, const std::string& controlLabel) override;
    uint32_t GetButtonID(const std::string& panelName, const std::string& buttonLabel) override;
    void SetControlValue(const std::string& JSON) override;
    bool WebSocketOpened(IWebSocketService* web_socket) override;

    // Constructs a control type, use similar to std::make_shared<>
    template <class CONTROL_TYPE, typename... ARGS>
    std::shared_ptr<CONTROL_TYPE> AddHapiControl(ARGS... args)
    {
        if (not _unassignedTab)
            _unassignedTab = AddUiTab("Unassigned Tab");

        return _unassignedTab->AddHapiControl<CONTROL_TYPE, ARGS...>(std::move(args)...);
    }

    std::shared_ptr<TopLevelUiTab> AddUiTab(const std::string& title,
                                            BooleanControlCB enabledCB = nullptr,
                                            const std::string& tooltip = "");
    void AddColumnBreak();

    void LogUiMessage(const std::string& msg);
    uint32_t ActiveConnectionCount();
    std::string GetURL(bool useHostname);

protected:
    void DoShutdown();
    void OsStartup();
    void ApplicationStartup();
    void ApplicationShutdown();
    std::shared_ptr<Settings> LoadSettingsFile(std::filesystem::path settingsFilename = "");
    void DelayStart();

    std::shared_ptr<AtUtils::MessageQueue> _spMessageQueue;

    static ICommonApplicationBase*  _pInstance;
    std::shared_ptr<ICommandMap>    _spCommandMap;
    std::shared_ptr<WebServer>      _spWebServer;
    std::shared_ptr<CommonUiLayer>  _spCommonUiLayer;
    std::shared_ptr<UiNotifier>     _spUiNotifier;
    std::shared_ptr<Notifier<int>> _spActivityNotifier;
    std::recursive_mutex            _spImageCapturerCS;
    std::shared_ptr<ImageNotifier>  _spImageNotifier;
    ButtonControlCB                 _updatePictureCB = nullptr;
    std::recursive_mutex            _settings_cs;
    std::shared_ptr<ISettings>       _spSettings;
    std::shared_ptr<IDialogFactory> _spDialogFactory;
    std::vector<std::weak_ptr<TopLevelUiTab>> _topLevelTabs;
    std::shared_ptr<TopLevelUiTab>  _unassignedTab;

    static AtUtils::Event _appShutdownEvent;
    std::shared_ptr<CommandLine> _spCommandLine;
    int         _webServerPort = 80;
    uint32_t    _windowWidth = 0; // Use screen size
    uint32_t    _windowHeight = 0; // Use screen size
    std::string _webFolder;
    std::string _productName;
    std::string _softwareVersion;
    bool        _debugging = false;
    bool        _waitAndShutdown = true;
    int         _startupDelaySeconds;
    std::string _validateClientID;
    UiConfig    _uiConfig;

    std::shared_ptr<TopLevelUiTab> _loggerTab;
    std::shared_ptr<LoggerControl> _logger;
};

