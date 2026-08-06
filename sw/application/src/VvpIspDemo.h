/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "CommonApplicationBase.h"
#include "CompileTimeConfiguration.h"
#include "Hapi.h"
#include "HapiOCS.h"
#include "HapiVvpWarp.h"
#include "UdxVvpIspPipeline.h"
#include "WebServer.h"
#ifndef ISP_STITCH_BUILD
#include "TabbedUiControls.h"
#else
#include "StitchPipeline.h"
#include "StitchTabbedUiControls.h"
#endif

#ifdef ISP_AI_BUILD
#include "AiPipeline.h"
#include "AiTabbedUiControls.h"
#endif


class VvpIspDemo : public CommonApplicationBase,
                   public Hapi::ILogCallback,
                   public IUIConnection
{
public:
    VvpIspDemo(int argument_count, char* argument_array[]);
    ~VvpIspDemo();

    static VvpIspDemo* Get();
    void StartShutdown() override;

    static const VvpIspCompileTimeConfiguration& GetCompileTimeConfig() 
    {
        return _VvpIspConfiguration;
    }

    bool Startup(StartupPhase phase) override;
    bool Shutdown(ShutdownPhase phase) override;
    std::vector<std::string> GetLogMessages() override;
    void UiConnected(uint32_t clientID) override;

    // Receive log message from HAPI system
    void LogMessage(std::string& message) override;
    void HandleCommandLineArgs();
    static std::ostream& PrintUsage(std::ostream& out = std::cout);

    void CaptureWarpOutput(uint32_t pipeline_index);
    bool ErrorMessage(const std::string& errorString);
    void ClearErrorMessage(const std::string& errorString);

    static void CallInitialisationShellScript();

#ifndef ISP_STITCH_BUILD
    std::shared_ptr<TabbedUiControls> GetUi() { return _spTabbedUi; }
#else
    std::shared_ptr<StitchTabbedUiControls> GetUi() { return _spTabbedUi; }
#endif

    bool UiConnected() override;
    std::string GetUiUrl(bool useHostname) override;

private:
    std::shared_ptr<Hapi::IHapi> _spHapi;

    bool _enableDebugUi = false;
    bool _powerUser = false;
    bool _showI2CAccesses = false;

    static VvpIspDemo* _pInstance;
    std::vector<std::string> _logMessages;
    std::string _ipAddress;
    std::vector<std::string> _errorStrings;

    std::shared_ptr<UdxVvpIspPipeline> _spUdxVvpIspPipeline;
#ifndef ISP_STITCH_BUILD
    std::shared_ptr<TabbedUiControls> _spTabbedUi;
#else
    std::shared_ptr<StitchPipeline> _spStitchPipeline;
    std::shared_ptr<StitchTabbedUiControls> _spTabbedUi;
#endif
#ifdef ISP_AI_BUILD
    std::shared_ptr<AiPipeline> _spAiPipeline;
    std::shared_ptr<AiTabbedUiControls> _spAiTabbedUi;
#endif
};
