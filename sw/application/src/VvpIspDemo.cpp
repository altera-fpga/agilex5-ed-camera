/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <algorithm>
#include <ctime>
#include <iostream>

#include "CommandLineFlags.h"
#include "CompileTimeConfiguration.h"
#include "VvpIspDemo.h"

// From HapiUI - these pull in the HAPI interfaces
#include "AccessLogging.h"
#include "LED.h"
#include "Logging.h"
#include "WebServer.h"
#include "version.h"

using namespace SwApi::AccessLogging;

static bool _appInitialised = false;

VvpIspDemo* VvpIspDemo::_pInstance = nullptr;

static std::string MakeSoftwareVersion()
{
    std::string softwareVersion = AtUtils::FormatString(
        "%d.%d.%d", version_major, version_minor, version_build);
    return softwareVersion;
}

VvpIspDemo::VvpIspDemo(int argumentCount, char* argumentArray[])
    : CommonApplicationBase(argumentCount,
                            argumentArray,
                            "Camera Solution",
                            MakeSoftwareVersion().c_str(),
                            GetCompileTimeConfig().STARTUP_DELAY_SECONDS)
{
    _pInstance = this;
}

VvpIspDemo::~VvpIspDemo()
{

}

VvpIspDemo* VvpIspDemo::Get()
{
    return _pInstance;
}

bool VvpIspDemo::Startup(StartupPhase phase)
{
    if (phase == StartupPhase::ONE)
    {
        // Look at our command line arguments, and load the flags we care about.
        HandleCommandLineArgs();

        CallInitialisationShellScript();

        bool showCapabilities = _spCommandLine->HaveOption("caps");

        _spHapi = Hapi::IHapiOCS::Create();
        if (!_spHapi)
        {
            return false;
        }

        if (showCapabilities)
        {
            _spHapi->LogDevices();
            return false;
        }
        _spHapi->LogDevices();

        // Register for log messages so we can capture them
        _spHapi->SetLogCallback(this);

        // Not interested in log messages after initialisation
        _spHapi->SetLogCallback(nullptr);

        _spUdxVvpIspPipeline = std::make_shared<UdxVvpIspPipeline>();
        _spUdxVvpIspPipeline->Initialize(_spHapi);
#ifdef ISP_AI_BUILD
        _spAiPipeline = std::make_shared<AiPipeline>(_spUdxVvpIspPipeline, _spHapi);
#endif
#ifdef ISP_STITCH_BUILD
        _spStitchPipeline = std::make_shared<StitchPipeline>(_spUdxVvpIspPipeline, _spHapi);
#endif
        _spUdxVvpIspPipeline->LinkPipelineCores();
#ifdef ISP_STITCH_BUILD
        _spStitchPipeline->LinkPipelineCores();
#endif

        auto createUiTabCB =
            [this](const std::string& title,
                   BooleanControlCB enabledCB,
                   const std::string& tooltip) -> std::shared_ptr<TopLevelUiTab>
            {
                // We use a lambda instead of passing the VvpIspDemo object over to prevent a
                // horrendous mess of circular header include and dependency issues.
                return AddUiTab(title, std::move(enabledCB), tooltip);
            };

        std::function<void(uint32_t)> warpCaptureCB = _enableDebugUi ? ([this] (uint32_t pipeline_index){
            CaptureWarpOutput(pipeline_index);
        }) : std::function<void(uint32_t)>{};

#ifndef ISP_STITCH_BUILD
        _spTabbedUi = std::make_shared<TabbedUiControls>(createUiTabCB, _spUdxVvpIspPipeline, _enableDebugUi, _powerUser, warpCaptureCB);
#else
        _spTabbedUi = std::make_shared<StitchTabbedUiControls>(createUiTabCB, _spUdxVvpIspPipeline, _spStitchPipeline, _enableDebugUi, _powerUser, warpCaptureCB);
#endif
        _spTabbedUi->CreateInputConfigTab();
        _spTabbedUi->CreateISPTab();
        _spTabbedUi->CreateOutputConfigTab();
        _spTabbedUi->CreateStatsTab();

        // Calibration only possible with the frame writer present
        if (_powerUser && _spUdxVvpIspPipeline->GetVfwCaptureIsp())
        {
            _spTabbedUi->CreateCalibrationTab();
        }

#ifdef ISP_AI_BUILD
        _spAiTabbedUi = std::make_shared<AiTabbedUiControls>(createUiTabCB, _spAiPipeline, _enableDebugUi, _powerUser);
        _spAiTabbedUi->CreateCoreDlaRuntimeTab();
#endif

#ifdef ISP_STITCH_BUILD
        _spTabbedUi->CreateStitchTab();
#endif

        if (_enableDebugUi)
        {
            _spTabbedUi->CreateDebugTab();
        }

        _spUdxVvpIspPipeline->LoadFuncsForUiControls(
            [this](const ImageConfig& params) -> void
            {
                return _spTabbedUi->UpdateInputParams(params);
            },
            [this](const ImageConfig& params) -> void
            {
                return _spTabbedUi->UpdateOutputParams(params);
            }
        );

        _spUdxVvpIspPipeline->LoadFuncsForUiFeedback(
            nullptr,
            [this](ImageConfig& params) -> void
            {
                _spTabbedUi->UpdateMixerParams(params);
            },
            [this](ImageConfig& config) -> void
            {
                _spTabbedUi->UpdateUiPostOutputChange(config);
            }
        );

        _appInitialised = true;
    }
    else if (phase == StartupPhase::TWO)
    {
        if (_appInitialised)
        {
            _spTabbedUi->EnableUIUpdates();
            _spTabbedUi->PostTabCreationUpdates();
            _spUdxVvpIspPipeline->Start();
#ifdef ISP_AI_BUILD
            _spAiPipeline->Start();
#endif
#ifdef ISP_STITCH_BUILD
            _spStitchPipeline->Start();
#endif
        }
    }

    return true;
}


bool VvpIspDemo::Shutdown(ShutdownPhase phase)
{
    if (phase == ShutdownPhase::ONE)
    {
        if (_spUdxVvpIspPipeline)
        {
            _spUdxVvpIspPipeline->Stop();
        }

#ifdef ISP_AI_BUILD
        if (_spAiPipeline)
        {
            _spAiPipeline->Stop();
        }
#endif

#ifdef ISP_STITCH_BUILD
        if (_spStitchPipeline)
        {
            _spStitchPipeline->Stop();
        }
#endif

        if (_showI2CAccesses)
        {
            WARN << "List of accesses:\n";

            // Print all recorded access logs
            TRACE << AccessLogger::Get().PrettyPrint() << '\n';
        }
    }

    return true;
}

std::vector<std::string> VvpIspDemo::GetLogMessages()
{
    return _logMessages;
}

// Receive log message from HAPI system
void VvpIspDemo::LogMessage(std::string& message)
{
    _logMessages.push_back(message);
}

void VvpIspDemo::StartShutdown()
{
    // Override StartShutdown to fade out video
    if (_spUdxVvpIspPipeline)
    {
        _spUdxVvpIspPipeline->StartShutdown();
    }
    CommonApplicationBase::StartShutdown();
}

bool VvpIspDemo::WebSocketOpened(IWebSocketService* web_socket)
{
    bool handled = false; // Return false to indicate that the application did not handle it
    for (auto& handler : _webSocketHandlers)
    {
        if(handler)
        {
            handled = handler(web_socket);
            if (handled)
            {
                break;
            }
        }
    }
    return handled;
}

size_t VvpIspDemo::RegisterWebSocketHandler(WebSocketOpenedCB web_socket_handler)
{
    _webSocketHandlers.push_back(web_socket_handler);
    return _webSocketHandlers.size() - 1;
}

void VvpIspDemo::UnRegisterWebSocketHandler(size_t web_socket_handler_handle)
{
    if (web_socket_handler_handle < _webSocketHandlers.size())
    {
        _webSocketHandlers[web_socket_handler_handle] = nullptr;
    }
}

///////////////////////

struct stop_watch_t
{
    stop_watch_t(const char* msg): _msg{msg}, _begin{std::chrono::steady_clock::now()} {}
    ~stop_watch_t(){
        auto end = std::chrono::steady_clock::now();
        long long t = std::chrono::duration_cast<std::chrono::milliseconds>(end - _begin).count();
        std::cout << _msg << ": " << std::dec << t << "[ms]" << std::endl;
    }

    const char* _msg;
    std::chrono::steady_clock::time_point _begin;
};

///////////////////////


void VvpIspDemo::CaptureWarpOutput(uint32_t pipeline_index)
{
    auto spWarpAdapter = _spUdxVvpIspPipeline->GetCoreIspPipeline(pipeline_index)->GetWarpAdapter();
    if (spWarpAdapter)
    {
        volatile stop_watch_t sw{"CAPTURE TIME, ms: "};

        auto spWarpAdapterPicture = spWarpAdapter->CapturePicture10();
        if (spWarpAdapterPicture)
        {
            auto spAppToolkitPicture
                = std::make_shared<AppToolkit::RawRgbImage16>(spWarpAdapterPicture->GetWidth(),
                                                              spWarpAdapterPicture->GetHeight(),
                                                              spWarpAdapterPicture->GetData());


            //sw = stop_watch_t{""};

            GetImageNotifier()->Notify(spAppToolkitPicture);
        }
    }
}

bool VvpIspDemo::ErrorMessage(const std::string& errorString)
{
    std::cout << errorString << std::endl;
    _errorStrings.emplace_back(std::move(errorString));
    return false;
}

void VvpIspDemo::ClearErrorMessage(const std::string& errorString)
{
    auto i = std::find(_errorStrings.begin(), _errorStrings.end(), errorString);
    if (i != _errorStrings.end())
    {
        _errorStrings.erase(i);
    }
}

void VvpIspDemo::UiConnected(uint32_t clientId)
{
    if (!_errorStrings.empty())
    {
        std::string errorString;

        for (auto& message : _errorStrings)
        {
            errorString += "<br/>";
            errorString += message;
        }
        auto showErrorCB = [errorString = std::move(errorString), clientId]()
            {
                UiMessage(errorString, nullptr, nullptr, clientId);
            };
        AddCommand(showErrorCB, 4000);
    }
}

void VvpIspDemo::HandleCommandLineArgs()
{
    // Usage/help flag
    if (CommandLineFlags::HaveOption(_spCommandLine, CommandLineFlags::UsageHelp))
    {
        PrintUsage();
        exit(0);
    }

    // Engineering/debug UI option
    _enableDebugUi = CommandLineFlags::HaveOption(_spCommandLine, CommandLineFlags::DebugUi);

    // Power user UI option
    _powerUser = CommandLineFlags::HaveOption(_spCommandLine, CommandLineFlags::PowerUser);

    // I2C access logging
    _showI2CAccesses = CommandLineFlags::HaveOption(_spCommandLine, CommandLineFlags::ShowI2CAccesses);

    // Printing compile time config
    if (CommandLineFlags::HaveOption(_spCommandLine, CommandLineFlags::PrintCompileConfig))
    {
        TRACE << this->GetCompileTimeConfig() << '\n';
        exit(0);
    }

}

static constexpr char ScriptPath[] = "./InitScriptVvpIsp.sh";

void VvpIspDemo::CallInitialisationShellScript()
{
    if (not system(nullptr))
    {
        WARN << "Could not get access to a shell, meaning we won't be able to run your start-up "
                "script. Do you have /bin/sh?\n";
        return;
    }

    auto fd = fopen(ScriptPath, "r");
    if (fd == nullptr)
    {
        TRACE << "No initialisation shell script was found at ''" << ScriptPath << "'. Skipping.\n'";
        return;
    }
    fclose(fd);

    auto err = system(ScriptPath);

    if (err == -1)
    {
        perror("Error during call to initalisation shell script.\n");
        ERR << "Failed to call init script at ('" << ScriptPath << "')\n";
        return;
    }
}

std::ostream& VvpIspDemo::PrintUsage(std::ostream& out)
{
    return out <<
      "\nUsage:"

        "\n\t-caps : Print details of the FPGA IP on the board (from the Omnitek "
                     "Capability Structure) and exit."

        "\n\t-debug-ui : Enable 'Engineering UI' in the web interface, enabling "
                        "access to (normally hidden) settings."

        "\n\t-show-i2c-accesses : Print a log of the I2C reads/writes done by the app on exit."
/*
        "\n\t-rgb : Selects which input the RGB Switch will pass through to its output. Choices:"
            "\n\t\t1 : Use RGB Switch input 1."
            "\n\t\t2 : Use RGB Switch input 2."
        "\n\t-bayer : Selects which input the Bayer Switch will pass through to its output. Choices:"
            "\n\t\t1 : Use Bayer Switch input 1."
            "\n\t\t2 : Use Bayer Switch input 2."
        "\n\t-cam-input : Choose input for camera module or on-board camera TPG. Choices:"
            "\n\t\ttpg1 or 1 : Use pattern 1."
            "\n\t\ttpg2 or 2 : Use pattern 2."
            "\n\t\ttpg3 or 3 : Use pattern 3."
            "\n\t\ttpg4 or 4 : Use pattern 4."
            "\n\t\tcam : Use the camera."
        "\n\t-cam-res : Choose input resolution for camera module or on-board camera TPG. Choices:"
            "\n\t\t1080p "
            "\n\t\t920p "
            "\n\t\t720p "
            "\n\t\t920s : 1632x920 Sub "
            "\n\t\t720s : 1280x720 Sub "
*/

        "\n\t-" << CommandLineFlags::IgnoreFailedHdmiInit <<
                   " : Continue even if HDMI subsystem is missing/fails to initialise."
        "\n\t-" << CommandLineFlags::IgnoreFailedCoreInit <<
                   " : Continue even if an IP core is missing/fails to initialise."
        "\n\t-" << CommandLineFlags::PrintCompileConfig <<
                   " : Print the configuration compiled into the app, and exit."

        "\n\tExamples:"
        /*
            "\n\t\t`./VvpIspDemo -cam-input=tpg4 -cam-res=720s`"
            "\n\t\t`./VvpIspDemo -cam-input=cam -cam-res=1080p`"
            "\n\t\t`./VvpIspDemo -cam-res=720p`"
            "\n\t\t`./VvpIspDemo -rgb=1 -bayer=1`"
        */
            "\n\t\t`./VvpIspDemo -debug-ui -show-i2c-accesses`"
      "\n"
      "\nInitialisation shell script:\n"
        "\n\tYou can place a shell script in '" << ScriptPath << "'. "
            "This will be called before any of the cores are initialised. "
        "\n\tYou can use this to add any devmem commands you'd like to run before setting up the pipeline."
        "\n\n";
}

bool VvpIspDemo::UiConnected()
{
    return ActiveConnectionCount() > 0;
}

std::string VvpIspDemo::GetUiUrl(bool useHostname)
{
    return GetURL(useHostname);
}