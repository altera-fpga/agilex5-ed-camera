/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "CommonApplicationBase.h"
#include "ICommand.h"
#include "WebServer.h"
#include "CommonUiLayer.h"
#include "CommonUiUpdate.h"
#include "CommonCommands.h"
#include "Settings.h"
#include "UiWebSocketCommandProcessor.h"
#include "LoggerControl.h"

#ifdef WIN32
#include <mmsystem.h>
#include <VersionHelpers.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/utsname.h>
#include <signal.h>
#endif

#if !defined(UNIT_TEST)
#define USE_EMBEDDED_WEB_FILES
#endif

ICommonApplicationBase* CommonApplicationBase::_pInstance = nullptr;
AtUtils::Event CommonApplicationBase::_appShutdownEvent;
uint32_t TopLevelUiTab::_nextTabIndex = 0;

CommonApplicationBase::CommonApplicationBase(int argumentCount, char* argumentArray[],
                                             const char* productName,
                                             const char* softwareVersion,
                                             int startupDelaySeconds)
:   _spUiNotifier{std::make_shared<Notifier<UiUpdate>>()}
,   _spActivityNotifier{std::make_shared<Notifier<int>>()}
,   _spDialogFactory{std::make_shared<DialogFactory>()}
,   _productName{productName}
,   _softwareVersion{softwareVersion}
,   _startupDelaySeconds{startupDelaySeconds}
{
    if (_pInstance)
        std::cout << "CommonApplicationBase::CommonApplicationBase already have a _pInstance\n";

    _pInstance = this;

    try { std::locale::global(std::locale("")); } catch (...) {}

    _spCommandLine = std::make_shared<CommandLine>(argumentCount, argumentArray);
    _spCommandMap = std::make_shared<CommandMap>();
    _spMessageQueue = std::make_shared<AtUtils::MessageQueue>("CommonApplicationBaseQ");

    OsStartup();
}

CommonApplicationBase::~CommonApplicationBase()
{
    _spMessageQueue.reset();
    _pInstance = nullptr;
    _unassignedTab.reset();
}

ICommonApplicationBase* CommonApplicationBase::Get()
{
    return _pInstance;
}

void CommonApplicationBase::SetInstance(ICommonApplicationBase* pInstance)
{
    _pInstance = pInstance;
}

int CommonApplicationBase::Run(bool waitAndShutdown)
{
    _waitAndShutdown = waitAndShutdown;
    std::time_t result = std::time(nullptr);
    char timeString[0x101]{};
    std::tm *theLocalTime = std::localtime(&result);
    if (theLocalTime)
        std::strftime(timeString, sizeof(timeString) - 1, "%c", theLocalTime);
    std::cout << "## " << _productName << " V" << _softwareVersion << " starting " << timeString << " ##" << std::endl;

    DelayStart();
    ApplicationStartup();

    if (!_appShutdownEvent.IsSignalled())
    {
        std::cout << "Ready.\n";

        std::string hostnameURL = _spWebServer->GetURL(true);
        std::string ipAddressURL = _spWebServer->GetURL(false);
        if (hostnameURL.empty())
        {
            std::cout << "Connect your browser to " << ipAddressURL << std::endl;
        }
        else
        {
            std::cout << "Connect your browser to " << hostnameURL << std::endl;
            std::cout << "or " << ipAddressURL << std::endl;
        }
        std::cout << "Type Ctrl+C to exit." << std::endl;
    }

    if (_waitAndShutdown)
    {
        _appShutdownEvent.Wait();
        DoShutdown();
    }

    return 0;
}

void CommonApplicationBase::DoShutdown()
{
    ApplicationShutdown();
    std::cout << _productName << " is now shutdown.\nAdios\n\n";
}

ICommandMap* CommonApplicationBase::GetCommandMap()
{
    return _spCommandMap.get();
}

AtUtils::MessageQueue* CommonApplicationBase::GetMessageQueue()
{
    return _spMessageQueue.get();
}

std::vector<std::weak_ptr<TopLevelUiTab>> CommonApplicationBase::GetTopLevelTabs()
{
    return _topLevelTabs;
}

std::shared_ptr<IDialogFactory> CommonApplicationBase::GetDialogFactory()
{
    return _spDialogFactory;
}

void CommonApplicationBase::AddCommand(AtUtils::Message* pCommand, uint32_t deltaTime)
{
    auto pMessageQueue = GetMessageQueue();
    if (pMessageQueue)
        pMessageQueue->Add(pCommand, deltaTime);
}

// Add callback to be executed once
void CommonApplicationBase::AddCommand(AtUtils::CommandQueueCB commandCB, uint32_t deltaTime)
{
    auto pMessageQueue = GetMessageQueue();
    if (pMessageQueue)
        pMessageQueue->Add(std::move(commandCB), deltaTime);

    int data = 0;
    _spActivityNotifier->Notify(data);
}

// Add a callback to be executed repeatedly
void CommonApplicationBase::AddCommand(AtUtils::AutoRepeatCommandQueueCB commandCB, uint32_t deltaTime)
{
    auto pMessageQueue = GetMessageQueue();
    if (pMessageQueue)
        pMessageQueue->Add(std::move(commandCB), deltaTime);
}

// Add a callback to be executed repeatedly
void CommonApplicationBase::AddCommand(AtUtils::VariableRepeatCommandQueueCB commandCB, uint32_t deltaTime)
{
    auto pMessageQueue = GetMessageQueue();
    if (pMessageQueue)
        pMessageQueue->Add(std::move(commandCB), deltaTime);
}

void CommonApplicationBase::RefreshUiControls()
{
    // This is called via callbacks from controls so do the
    // work in a command as this will tear down the stack of
    // objects that called the callback
    auto refreshUiControlsCB = []() { CommonUiLayer::Get()->RefreshUiControls(); };
    AddCommand(refreshUiControlsCB);
}

std::shared_ptr<UiNotifier> CommonApplicationBase::GetUiNotifier()
{
    return _spUiNotifier;
}

std::shared_ptr<ImageNotifier> CommonApplicationBase::GetImageNotifier()
{
    // Lock the creation of ImageNotifier since this could be called
    // from PictureWebSocketCommandProcessor or caller of GetImageNotifier
    // in the application code
    std::lock_guard lock(_spImageCapturerCS);

    if (!_spImageNotifier)
        _spImageNotifier = std::make_shared<Notifier<std::shared_ptr<AppToolkit::RawRgbImage16>>>();

    return _spImageNotifier;
}

std::shared_ptr<INotifier<int>> CommonApplicationBase::GetActivityNotifier()
{
    return _spActivityNotifier;
}

void CommonApplicationBase::StartShutdown()
{
    ShutdownApp();
}

void CommonApplicationBase::ShutdownApp()
{
    _appShutdownEvent.Set();
}

void CommonApplicationBase::DelayStart()
{
    if (_spCommandLine->HaveOption("caps"))
        _startupDelaySeconds = 0;

    if (_spCommandLine->HaveOption("startup_delay"))
    {
        AtUtils::FromString(_spCommandLine->GetOptionValue("startup_delay"), _startupDelaySeconds);
        _startupDelaySeconds = std::min(_startupDelaySeconds, 9);
    }

    if (_startupDelaySeconds > 0)
    {
        using namespace std::chrono_literals;
        std::cout << "Waiting   " << std::flush;
        do
        {
            std::cout << "\b\b" << _startupDelaySeconds << "s" << std::flush;
            std::this_thread::sleep_for(1s);
        } while (--_startupDelaySeconds > 0);
        std::cout << "\b\b\b\b\b\b\b\b\b\b" << std::flush;
    }
}

void CommonApplicationBase::ApplicationStartup()
{
    if (_spCommandLine->HaveOption("port"))
        AtUtils::FromString(_spCommandLine->GetOptionValue("port"), _webServerPort);

    _webFolder = _spCommandLine->GetOptionValue("web_folder");

    // Stops the UI timing out when debugging
    if (_spCommandLine->HaveOption("debug"))
        _debugging = true;

    auto uiConnectedCB = [this](uint32_t clientID) { UiConnected(clientID); };

    _spWebServer = std::make_shared<WebServer>(_webServerPort, _webFolder, _validateClientID, this);
    _spCommonUiLayer = std::make_shared<CommonUiLayer>(uiConnectedCB);
    _spSettings = LoadSettingsFile();

    // If a web folder is specified on the command line, don't enable the
    // embedded web files
    if (_webFolder.empty())
    {
#ifdef USE_EMBEDDED_WEB_FILES
        char* pBinaryBlobStart = nullptr;
        char* pBinarayBlobEnd = nullptr;

#ifdef WIN32
        HRSRC binaryResource = ::FindResourceA(NULL, MAKEINTRESOURCE(100), RT_RCDATA);
        if (binaryResource != NULL)
        {
            HGLOBAL resourceHandle = ::LoadResource(NULL, binaryResource);
            if (resourceHandle != NULL)
            {
                void* pResource = LockResource(resourceHandle);
                if (pResource)
                {
                    DWORD resourceSize = SizeofResource(NULL, binaryResource);

                    pBinaryBlobStart = static_cast<char*>(pResource);
                    pBinarayBlobEnd = pBinaryBlobStart + resourceSize;
                }
            }
        }
#else
        extern char _binary_WebServerHome_bin_start[];
        extern char _binary_WebServerHome_bin_end[];
        pBinaryBlobStart = _binary_WebServerHome_bin_start;
        pBinarayBlobEnd = _binary_WebServerHome_bin_end;
#endif

        if (pBinaryBlobStart != nullptr)
            _spWebServer->UseEmbeddedFiles(pBinaryBlobStart, pBinarayBlobEnd);
#endif
    }

    bool startupFailed = false;

    for (StartupPhase phase : { StartupPhase::ONE, StartupPhase::TWO })
    {
        if (!_spCommonUiLayer->Startup(phase))
            startupFailed = true;

        if (!_spWebServer->Startup(phase))
            startupFailed = true;

        // Startup derived class
        if (!Startup(phase))
            startupFailed = true;

        //////////////////////////
        static constexpr bool enableLogging = false;

        if(enableLogging and phase == StartupPhase::ONE)
        {
            _loggerTab = AddUiTab("Logger");
            _logger = _loggerTab->AddHapiControl<LoggerControl>();
        }

        //////////////////////////            
    }

    if (startupFailed)
    {
        // No device or failed to startup, so shutdown
        StartShutdown();
    }
    else
    {
        // Start auto save
        static constexpr uint32_t checkAutoSaveInterval = 5000;
        auto checkAutoSaveCB = [this](uint32_t&)
        {
            // Safely lock the settings and check if it needs saving
            std::shared_ptr<ISettings> spSettings;
            {
                std::lock_guard lock(_settings_cs);
                spSettings = _spSettings;
            }

            if (spSettings->IsDirty())
                spSettings->Save();

            return true;
        };
        AddCommand(checkAutoSaveCB, checkAutoSaveInterval);

        // Send a regular message to clients, so they know when the
        // server is dead/disconnected
        static constexpr uint32_t pingClientInterval = 4111; // Some prime number to mix timing in with others
        auto pingClientCB = [](uint32_t& counter) { UiUpdate::PingUiClients((uint32_t)counter); return true; };
        AddCommand(pingClientCB, pingClientInterval);
    }
}

void CommonApplicationBase::LogUiMessage(const std::string& msg)
{
    //UiUpdate::SendLoggerMessage(msg);
    //_logger->LogMessage(msg);
}

uint32_t CommonApplicationBase::ActiveConnectionCount()
{
    return _spWebServer ? _spWebServer->ActiveConnectionCount() : 0; 
}

std::string CommonApplicationBase::GetURL(bool useHostname)
{
    std::string url;
    if (_spWebServer)
    {
        url = _spWebServer->GetURL(useHostname);
    }
    return url;
}

void CommonApplicationBase::ApplicationShutdown()
{
    std::cout << "Shutting down\n";

    for (ShutdownPhase phase : { ShutdownPhase::ONE, ShutdownPhase::TWO })
    {
        _spWebServer->Shutdown(phase);
        _spCommonUiLayer->Shutdown(phase);

        // Shutdown derived class
        Shutdown(phase);

        if (phase == ShutdownPhase::ONE)
        {
            // Shutdown the message queue
            _spMessageQueue.reset();
        }
    }

    if (_spSettings->IsDirty())
        _spSettings->Save(true);
}

std::shared_ptr<Settings> CommonApplicationBase::LoadSettingsFile(std::filesystem::path settingsFilename)
{
    if (settingsFilename.string().empty())
    {
        // At startup, modify the settings filename if port is different
        // from 80 or 8080, so multiple servers on different ports can 
        // have their own settings
        if ((_webServerPort != 80) and (_webServerPort != 8080))
        {
            uint32_t settingsIndex = 0;
            if (_webServerPort > 8080)
                settingsIndex = _webServerPort - 8080;
            else
                settingsIndex = _webServerPort - 80;

            Settings::_settingsFileName = AtUtils::FormatString("settings-%d.json", settingsIndex);
        }
        
        settingsFilename = Settings::_settingsFileName;
    }
    auto spSettings = std::make_shared<Settings>(settingsFilename);
    return spSettings;
}

int CommonApplicationBase::NewFileImportStarted(std::string& filename, size_t index,
                                                 size_t numFiles, bool isSequence,
                                                 uint32_t clientID)
{
    return 0;
}

void CommonApplicationBase::LoadNewSettings(std::shared_ptr<ISettings> spNewSettings)
{
    if (spNewSettings)
    {
        _spSettings = std::move(spNewSettings);

        // Set flag on the settings container so that
        // settings are not written during a load
        _spSettings->SetLoading(true);

        // Push to the UI, each control will callback the app with the new settings
        _spCommonUiLayer->LoadSettings(false);

        _spSettings->SetLoading(false);
    }
}

void CommonApplicationBase::ImportFile(std::shared_ptr<WebSocketFileTransfer> spWsFileTransfer)
{
    size_t nFiles = spWsFileTransfer->GetNumFiles();

    if (nFiles > 0)
    {
        auto importFilename = spWsFileTransfer->at(0)->GetFileName();

        // Call the import action callback function on main command queue
        AddCommand([imp = std::move(importFilename)]() { ImportFileUiUpdate::NewFileReceived(imp); });
    }
}

void CommonApplicationBase::RestoreFromBackup()
{
    auto spNewSettings = _spSettings->RestoreFromBackup();
    _spSettings = std::move(spNewSettings);

    // Push to the UI, each control will callback the app with the new settings
    CommonApplicationBase::Get()->LoadNewSettings(_spSettings);
}

void CommonApplicationBase::RestoreFactorySettings()
{
    _spSettings = _spSettings->RestoreFactorySettings();

    // Push to the UI, each control will callback the app with the new settings
    _spCommonUiLayer->LoadSettings(true);
}

// IWebAPI implementation

std::string CommonApplicationBase::GetControlsJSON()
{
    JsonDOM jsonDOM("WebAPI");
    AtUtils::IJsonObjectPtr spUiJsonObject = jsonDOM.GetRoot();
    _spCommonUiLayer->GetUiControlsFlat(spUiJsonObject);
    return *jsonDOM.ToString();     
}

std::string CommonApplicationBase::GetControlJSON(uint32_t controlID)
{
    std::shared_ptr<UiElement> spControl = _spCommonUiLayer->GetControl(controlID);
    if (!spControl)
        return "";

    JsonDOM jsonDOM("Control");
    AtUtils::IJsonObjectPtr spJsonObject = jsonDOM.GetRoot();
    spControl->GetJSON(spJsonObject);
    return *jsonDOM.ToString();     
}

uint32_t CommonApplicationBase::GetControlID(const std::string& panelName, const std::string& controlLabel)
{
    uint32_t controlID = 0;

    std::shared_ptr<UiElement> spControl = _spCommonUiLayer->GetControl(panelName, controlLabel);
    if (spControl)
        controlID = spControl->GetID();
    else
        std::cout << "Control not found for panel '" << panelName << "' and label '" << controlLabel << "'\n";

    return controlID;
}

uint32_t CommonApplicationBase::GetButtonID(const std::string& panelName, const std::string& buttonLabel)
{
    uint32_t controlID = 0;

    std::shared_ptr<UiElement> spControl = _spCommonUiLayer->GetButton(panelName, buttonLabel);
    if (spControl)
        controlID = spControl->GetID();
    else
        std::cout << "Button not found for panel '" << panelName << "' and label '" << buttonLabel << "'\n";

    return controlID;
}

void CommonApplicationBase::SetControlValue(const std::string& JSON)
{
    auto spJson = AtUtils::IJson::Create(JSON);
    if (spJson)
    {
        auto spObject = spJson->Parse();

        auto spUniqueIdObject = spObject->GetValue("unique_id");
        if (!spUniqueIdObject)
        {
            return;
        }
        AtUtils::JsonValueVariant valueVariant = spUniqueIdObject->GetValue();

        uint32_t uniqueID = 0;
        if (std::holds_alternative<std::string>(valueVariant))
            AtUtils::FromString(spUniqueIdObject->GetValue<std::string>(), uniqueID);
        else
            uniqueID = spObject->GetValue<uint32_t>("unique_id");

        std::string commandValueString = "ControlItemUpdate_" + std::to_string(uniqueID);

        auto spCommandValue = spObject->GetValue("value");
        if (!spCommandValue)
        {
            return;
        }
        valueVariant = spCommandValue->GetValue();

        std::string paramsValueString;

        if (std::holds_alternative<uint32_t>(valueVariant))
        {
            uint32_t uint32Value = spCommandValue->GetValue<uint32_t>();
            paramsValueString = std::to_string(uint32Value);
        }
        else if (std::holds_alternative<int32_t>(valueVariant))
        {
            int32_t int32Value = spCommandValue->GetValue<int32_t>();
            paramsValueString = std::to_string(int32Value);
        }
        else if (std::holds_alternative<std::string>(valueVariant))
        {
            paramsValueString = spCommandValue->GetValue<std::string>();
        }
        else if (std::holds_alternative<double>(valueVariant))
        {
            double doubleValue = spCommandValue->GetValue<double>();
            paramsValueString = std::to_string(doubleValue);
        }
        else if (std::holds_alternative<bool>(valueVariant))
        {
            bool boolValue = spCommandValue->GetValue<bool>();
            paramsValueString = boolValue ? "true" : "false";
        }

        uint32_t clientID = UiWebSocketCommandProcessor::GetLastClientID();
        auto paramList = std::make_shared<ParamList>(paramsValueString, clientID);
        paramList->SetBaseCommand(commandValueString);

        ICommonApplicationBase *pCoreApp = CommonApplicationBase::Get();
        ICommandMap *pServerCommandMap = pCoreApp->GetCommandMap();
        UiServerCommand *pServerCommand = pServerCommandMap->LookupCommand(commandValueString);

        if (pServerCommand)
        {
            // Found a system wide command

            // Delegate to the main application message queue so all commands from
            // different clients get serialized
            auto executeOnAppQueueCB = [pServerCommand, c_paramList = std::move(paramList)]()
            {
                pServerCommand->Execute(c_paramList);
            };
            pCoreApp->AddCommand(executeOnAppQueueCB);
        }
    }
}

bool CommonApplicationBase::WebSocketOpened(IWebSocketService* web_socket)
{
    // default websocket is not handled by the application, so return false to indicate that the application did not handle it
    return false;
}


std::shared_ptr<TopLevelUiTab> CommonApplicationBase::AddUiTab(const std::string& title,
                                                               BooleanControlCB enabledCB,
                                                               const std::string& tooltip)
{
    if (_topLevelTabs.empty())
    {
        // Register callback command for tab enable
        ICommandMap* pCommandMap = CommonApplicationBase::Get()->GetCommandMap();
        auto enabledParamCB = [this](ParamListPtr& spParameterList)
        {
            if (spParameterList->CheckNumParams(2))
            {
                uint32_t clientID = spParameterList->GetClientID();
                int32_t selectedIndex = 0;
                int32_t deselectedIndex = 0;
                AtUtils::FromString(spParameterList->GetParam(0), selectedIndex);
                AtUtils::FromString(spParameterList->GetParam(1), deselectedIndex);

                if (selectedIndex != deselectedIndex)
                {
                    if ((deselectedIndex >= 0) and (deselectedIndex < (int32_t)_topLevelTabs.size()))
                    {
                        std::shared_ptr<TopLevelUiTab> spDeselectedTab = _topLevelTabs[deselectedIndex].lock();
                        spDeselectedTab->CallCallback(clientID, false);
                    }
                }

                if ((selectedIndex >= 0) and (selectedIndex < (int32_t)_topLevelTabs.size()))
                {
                    std::shared_ptr<TopLevelUiTab> spSelectedTab = _topLevelTabs[selectedIndex].lock();
                    spSelectedTab->CallCallback(clientID, true);
                }
            }
        };

        pCommandMap->Add(new UiServerCommand("TopLevelTabEnabled", enabledParamCB));
    }

    auto spUiTab = std::make_shared<TopLevelUiTab>(title, tooltip, enabledCB);
    _topLevelTabs.push_back(spUiTab);
    return spUiTab;
}

void CommonApplicationBase::AddColumnBreak()
{
    if (_unassignedTab)
    {
        _unassignedTab->AddColumnBreak();
    }
}

#ifdef WIN32

BOOL _stdcall ControlHandler(DWORD controlType)
{
    switch (controlType)
    {
    case CTRL_C_EVENT:
        std::cout << "ControlHandler CTRL_C_EVENT\n";
        CommonApplicationBase::Get()->StartShutdown();
        return TRUE;

    default:
        return FALSE;
    }
}

// Windows specific application startup and shutdown code
void CommonApplicationBase::OsStartup()
{
    ::timeBeginPeriod(1);
    WSADATA info;
    if (WSAStartup(MAKEWORD(2, 0), &info))
    {
        throw "Could not start WSA";
    }
    BOOL r = ::SetConsoleCtrlHandler(ControlHandler, TRUE);
    (void)r;

}

#endif

#ifdef __linux__
// Linux specific application startup and shutdown code

void SigIntHandler(int s)
{
    std::cout << "\nCtrl+C detected. Shutting down application\n";
    CommonApplicationBase::Get()->StartShutdown();
}

void CommonApplicationBase::OsStartup()
{
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigprocmask(SIG_BLOCK, &signal_set, NULL);

    // Ctrl+C will exit the application
    signal(SIGINT, SigIntHandler);
    signal(SIGUSR1, SigIntHandler);
    signal(SIGPIPE, SIG_IGN);
}

#endif
