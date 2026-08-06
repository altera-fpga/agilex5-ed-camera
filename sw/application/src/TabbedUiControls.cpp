/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "CommonApplicationBase.h"
#include "TabbedUiControls.h"

#include <ranges>
#include "CoeffGen.h"
#include "VvpIspDemo.h"



TabbedUiControls::TabbedUiControls(std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> appCreateUiTabCB,
                                    std::shared_ptr<UdxVvpIspPipeline> spPipeline, bool debugMode, bool powerUser, std::function<void(uint32_t)> warpCaptureCB)
    : _appCreateUiTabCB(appCreateUiTabCB),
      _spTaskHandler{std::make_shared<TaskHandler>()},
      _spPipeline(spPipeline),
      _debugMode(debugMode),
      _powerUser(powerUser),
      _warpCaptureCB(warpCaptureCB),
      _spActiveCameraUiControls(nullptr),
      _autoExpEnableState{false},
      _aeTaskHandle{std::numeric_limits<std::size_t>::max()},
      _awbTaskHandle{std::numeric_limits<std::size_t>::max()}
{
    _uiInitialised = false;
}

TabbedUiControls::~TabbedUiControls()
{
    if (_histogramPreviewSubscriptionId == 0)
        return;

    auto spHs = _wpHistogramPreviewSource.lock();

    if (spHs)
    {
        spHs->UnregisterHistogramObserver(_histogramPreviewSubscriptionId);
    }
}

void TabbedUiControls::EnableUIUpdates()
{
    _spTaskHandler->Start();
    _uiInitialised = true;
}


std::shared_ptr<VvpIspSourceSelectControls> TabbedUiControls::CreateInputSelector()
{
    // ISP video inputs user friendly names as displayed in UI
    static const std::map<PipelineInput, std::string> input_names = {
        {PipelineInput::TPG,        "Input TPG"},
        {PipelineInput::Camera0,    "Camera 0"},
        {PipelineInput::Camera1,    "Camera 1"},
        {PipelineInput::Cameras,    "Cameras"},
        {PipelineInput::FR,         "Frame Reader"},
        {PipelineInput::Invalid,    "Unknown"}
    };

    auto video_inputs = _spPipeline->GetVideoInputs();
    std::vector<std::string> inputOpts{};

    for(const auto& vi: video_inputs)
    {
        auto p = input_names.find(vi);

        if(p != input_names.end())
            inputOpts.emplace_back(p->second);
    }

    auto inputSelectorCb = [this](const uint32_t inputIdx){

        bool switchOk = _spPipeline->SelectInput(inputIdx);

        if(switchOk)
        {
            const auto& cameras = _spPipeline->GetCameras();
            const std::size_t numCams = cameras.size();

            PipelineInput selected_input = _spPipeline->GetUiSelectedInput();
            std::size_t mipi_index = numCams;

            switch(selected_input)
            {
                case PipelineInput::Camera0:
                    mipi_index = 0;
                break;
                case PipelineInput::Camera1:
                    mipi_index = 1;
                break;
                default:
                break;
            }

            // Stop update handlers before changing the active camera
            bool aeEnabled = false;

            if(_spAutoWhiteBalanceControls)
            {
                _spTaskHandler->DisableTask(_awbTaskHandle);
            }

            if(_spAutoExposureControls)
            {
                _spAutoExposureControls->Enable(false);
                aeEnabled = _spAutoExposureControls->AeEnabled();

                if(aeEnabled)
                    _spTaskHandler->DisableTask(_aeTaskHandle);
            }

            // Set active camera controls
            if(_spActiveCameraUiControls)
                _spActiveCameraUiControls->SetActive(false);

            _spActiveCameraUiControls = (mipi_index < _cameraControls.size()) ? _cameraControls[mipi_index] : nullptr;

            auto coreIspPipeline = _spPipeline->GetCoreIspPipeline();

            // This will update AutoExposure and WhiteBalance controllers
            coreIspPipeline->SetActiveCamera((mipi_index < numCams) ? cameras[mipi_index] : nullptr);

            if(_spActiveCameraUiControls)
                _spActiveCameraUiControls->SetActive(true);

            // Update AWB Black Level after changing cameras
            // If Auto Exposure is enabled it will be overwritten
            if(_spAutoWhiteBalanceControls && _spAutoWhiteBalanceControls->IsAwbEnabled())
                coreIspPipeline->GetWhiteBalanceController()->ApplyBlackLevel();

            // Re-enable handlers after the camera has been changed
            if (_spAutoExposureControls)
            {
                const bool enableAutoExposureControls = (mipi_index < numCams);

                if(enableAutoExposureControls)
                {
                    if(aeEnabled)
                        _spTaskHandler->EnableTask(_aeTaskHandle);
                }

                _spAutoExposureControls->Enable(enableAutoExposureControls);
            }

            if(_spAutoWhiteBalanceControls)
            {
                _spTaskHandler->EnableTask(_awbTaskHandle);
            }
        }
    };

    return _spInputConfigTab->AddHapiControl<VvpIspSourceSelectControls>("Input Source", std::move(inputOpts), inputSelectorCb);
}


void TabbedUiControls::CreateInputConfigTab()
{
    auto inputConfigControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
        if (enabled)
        {
            // Do nothing
        }
        else
        {
            // Disable ROI highlighting and release TMO override
            _spAutoWhiteBalanceControls->DisableRoiHighlighting();
            _spAutoExposureControls->DisableRoiHighlighting();
        }
    };

    _spInputConfigTab = _appCreateUiTabCB("Input Config", inputConfigControlsEnabledCB, "Controls for configuring the input to the ISP pipeline.");

    const auto cameras = _spPipeline->GetCameras();
    const auto exposureFusions = _spPipeline->GetExposureFusions();

    // Input selector
    _spSourceSelector = CreateInputSelector();

    // TPG
    auto spTpg = _spPipeline->GetTpg();

    _spTpgControls = _spInputConfigTab->AddHapiControl<TpgControls>(spTpg, true);
    _spTpgControls->RegisterOnImageSettingsChangeCb(
            [this]() -> void {
                _spPipeline->ReprogramPipeAction(UdxVvpIspPipeline::PipelineStateChanged::PreWarp);
            });

    static constexpr uint32_t TPG_RAINBOW_INTERVAL_MS = 100;

    std::size_t task_idx = _spTaskHandler->CreateTask([this](){
        _spTpgControls->RainbowUpdateLoop();
    }, TPG_RAINBOW_INTERVAL_MS);

    _spTaskHandler->EnableTask(task_idx);

    // Cameras
    if (!cameras.empty())
    {
        std::size_t idx = 0;

        for(const auto& c: cameras)
        {
            const std::string camera_name = "Camera " + std::to_string(idx);
            const auto mipi_index = c->GetMIPIInterface();

            _cameraControls.emplace_back(_spInputConfigTab->AddHapiControl<CameraUiControls>(c, camera_name, exposureFusions[idx], _debugMode || _powerUser));

            auto analogueGainCb = [this, mipi_index](const float v)
            {
                auto coreIspPipeline = _spPipeline->GetCoreIspPipeline();
                auto activeCamera = coreIspPipeline->GetActiveCamera();
                if(activeCamera)
                {
                    if(mipi_index == activeCamera->GetMIPIInterface())
                    {
                        coreIspPipeline->GetWhiteBalanceController()->SetGain(v);
                    }
                }

                if(_spAutoWhiteBalanceControls->IsAwbEnabled())
                {
                    coreIspPipeline->GetWhiteBalanceController()->ApplyBlackLevel();
                }
            };

            _cameraControls.back()->SetAnalogueGainCb(analogueGainCb);
            idx++;
        }
    }
    else
    {
        WARN << "Camera module not detected by pipeline - will not add to UI.\n";
    }

    const auto videoInputsFr = _spPipeline->GetVideoInputs();
    if(std::ranges::find(videoInputsFr, PipelineInput::FR) != std::ranges::end(videoInputsFr))
    {
        _spFrameReaderControls = _spInputConfigTab->AddHapiControl<VvpIspFrameReaderControls>(_spPipeline->GetFrameReaderInput());      
    }    

    _spInputConfigTab->AddColumnBreak();

    auto coreIspPipeline = _spPipeline->GetCoreIspPipeline();

    const std::string profile_loader_name = "Sensor Profile";
    _spCameraProfileControls = _spInputConfigTab->AddHapiControl<VvpIspProfileLoaderControls>(
        profile_loader_name,
        coreIspPipeline->GetProfile(),
        coreIspPipeline->GetWhiteBalanceController()
    );

    // Histogram preview panel
    auto spHs = coreIspPipeline->GetHs();
    if (spHs)
    {
        const std::string histogram_name = "Histogram";
        _spHistogramPreview = _spInputConfigTab->AddHapiControl<HistogramPreview>(histogram_name);

        _wpHistogramPreviewSource = spHs;
        _histogramPreviewSubscriptionId = spHs->RegisterHistogramObserver([this](const std::shared_ptr<SwApi::Hs::TableResults>& results){
            if (_spHistogramPreview && results)
            {
                _spHistogramPreview->Update(*results);
            }
        });
    }

    // Clipper
    if(_debugMode || _powerUser)
    {
        auto spClipper = coreIspPipeline->GetClipper();

        if (spClipper)
        {
            _spClipperControls = _spInputConfigTab->AddHapiControl<ClipperControls>(spClipper, true);

            auto clipperUpdatePipelineCB = [this] (void)
            {
                _spPipeline->ReprogramPipeAction(UdxVvpIspPipeline::PipelineStateChanged::PreWarpNoClipperReset);
            };
            _spClipperControls->LoadUpdatePipelineCB(clipperUpdatePipelineCB);

            static constexpr uint32_t CLIPPER_INTERVAL_MS = 100;

            std::size_t clipper_task_idx = _spTaskHandler->CreateTask([this](){
                _spClipperControls->PolledUpdateCB();
            }, CLIPPER_INTERVAL_MS);

            _spTaskHandler->EnableTask(clipper_task_idx);
        }
    }

    _spInputConfigTab->AddColumnBreak();

    // AutoExposure
    auto spAutoExposure = coreIspPipeline->GetAutoExposureController();
    if (spAutoExposure)
    {
        auto roi = coreIspPipeline->GetROI();

        const std::string auto_exposure_name = "Auto Exposure";
        _spAutoExposureControls = _spInputConfigTab->AddHapiControl<VvpIspAutoExposureControls>(auto_exposure_name, spAutoExposure, roi, _debugMode, _powerUser);

        static constexpr uint32_t AUTOEXPOSURE_INTERVAL_MS = 100;

        std::size_t task_idx = _spTaskHandler->CreateTask([this, coreIspPipeline, spAutoExposure](){

            auto values_nearly_same = [](const float v1, const float v2)->bool {
                static constexpr float EPSILON = 0.0000001f;
                return std::fabs(v1 - v2) < EPSILON;
            };

            _spAutoExposureControls->AeTask();

            if (_spActiveCameraUiControls)
            {
                const float shutterSpeed = spAutoExposure->GetShutterSpeed();
                const float analogueGain = spAutoExposure->GetAnalogueGain();
                const float digitalGain = spAutoExposure->GetDigitalGain();

                if(!values_nearly_same(shutterSpeed, _spActiveCameraUiControls->GetShutterSpeed())){
                    _spActiveCameraUiControls->SetShutterSpeed(shutterSpeed);
                }

                if(!values_nearly_same(analogueGain, _spActiveCameraUiControls->GetAnalogueGain())){
                    _spActiveCameraUiControls->SetAnalogueGain(analogueGain);
                }

                if(!values_nearly_same(digitalGain, _spActiveCameraUiControls->GetDigitalGain())){
                    _spActiveCameraUiControls->SetDigitalGain(digitalGain);
                }
            }
        }, AUTOEXPOSURE_INTERVAL_MS);

        _aeTaskHandle = task_idx;

        auto autoExposureEnableCB = [this, task_idx](bool enable){
            enable ? _spTaskHandler->EnableTask(task_idx) : _spTaskHandler->DisableTask(task_idx);
        };

        _spAutoExposureControls->SetAutoUpdateToggleFunc(autoExposureEnableCB);
    }

    _spInputConfigTab->AddColumnBreak();

    auto roi = coreIspPipeline->GetROI();

    const std::string auto_white_balance_name = "Auto White Balance";
    _spAutoWhiteBalanceControls = _spInputConfigTab->AddHapiControl<VvpIspAutoWhiteBalanceControls>(
        auto_white_balance_name,
        coreIspPipeline->GetWhiteBalanceController(),
        roi
    );

    static constexpr uint32_t AUTO_WHITE_BALANCE_INTERVAL_MS = 100;

    std::size_t awb_task_idx = _spTaskHandler->CreateTask([this](){
         _spAutoWhiteBalanceControls->AutoWhiteBalanceUpdateFunc();
    }, AUTO_WHITE_BALANCE_INTERVAL_MS);

    _awbTaskHandle = awb_task_idx;

    _spTaskHandler->EnableTask(awb_task_idx);

    auto awbUpdateBLCAndWBCCB = [this, coreIspPipeline](bool override)
    {
        if (_spBlcControls)
        {
            _spBlcControls->BlockUIBasedOnBLSCallback(override);
        }
        if (_spWbcControls)
        {
            _spWbcControls->BlockUIBasedOnAWBCallback(override);
        }
        if (_spBlsControls)
        {
            auto spBlc = coreIspPipeline->GetBlc();
            auto pedestals = spBlc->GetBlackPedestals();
            _spBlsControls->UpdateGraphValues(pedestals[0], pedestals[1], pedestals[2], pedestals[3],
                                              (pedestals[0] + pedestals[1] + pedestals[2] + pedestals[3]) / 4);
        }
    };
    _spAutoWhiteBalanceControls->LoadBlockBlcWbcFunctionPointer(awbUpdateBLCAndWBCCB);

    auto awbSetBypassBLCAndWBCCB = [this](bool bypass)
    {
        if (_spBlcControls)
        {
            _spBlcControls->BypassBasedOnAWBCallback(bypass);
        }
        if (_spWbcControls)
        {
            _spWbcControls->BypassBasedOnAWBCallback(bypass);
        }
    };
    _spAutoWhiteBalanceControls->SetBypassBLCAndWBCFunctionPointer(awbSetBypassBLCAndWBCCB);
}

void TabbedUiControls::UpdateInputParams(const ImageConfig& inputConfig)
{
    if (!_uiInitialised)
        return;
}

void TabbedUiControls::CreateISPTab()
{
    auto ispControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    _spISPPipelineTab = _appCreateUiTabCB("ISP Pipeline", ispControlsEnabledCB, "Controls for configuring the ISP pipeline cores.");

    auto coreIspPipeline = _spPipeline->GetCoreIspPipeline();

    auto spDpc = coreIspPipeline->GetDpc();
    if (spDpc)
    {
        const std::string dpc_name = "Defective Pixel Correction";
        _spDpcControls = _spISPPipelineTab->AddHapiControl<DpcControls>(std::move(dpc_name), std::move(spDpc), _debugMode);
    }

    auto spAnr = coreIspPipeline->GetAnr();
    if (spAnr)
    {
        const std::string anr_name = "Adaptive Noise Reduction";
        _spAnrControls = _spISPPipelineTab->AddHapiControl<AnrControls>(std::move(anr_name), std::move(spAnr));
    }

    auto spBlc = coreIspPipeline->GetBlc();
    if (spBlc)
    {
        const std::string blc_name = "Black Level Correction";
        _spBlcControls = _spISPPipelineTab->AddHapiControl<BlcControls>(std::move(blc_name), std::move(spBlc), _debugMode, _powerUser);
    }

    auto spVc = coreIspPipeline->GetVc();
    if (spVc)
    {
        const std::string vc_name = "Vignette Correction";
        _spVcControls = _spISPPipelineTab->AddHapiControl<VcControls>(std::move(vc_name), std::move(spVc), _debugMode);
    }

    auto spWbc = coreIspPipeline->GetWbc();
    if (spWbc)
    {
        const std::string wbc_name = "White Balance Correction";
        _spWbcControls = _spISPPipelineTab->AddHapiControl<WbcControls>(std::move(wbc_name), std::move(spWbc), _debugMode);
    }

    auto spDemosaic = coreIspPipeline->GetDemosaic();
    if (spDemosaic)
    {
        const std::string demosaic_name = "Demosaic";
        _spDemosaicControls = _spISPPipelineTab->AddHapiControl<DemosaicControls>(std::move(demosaic_name), std::move(spDemosaic), _debugMode);
    }

    auto spCcm = coreIspPipeline->GetCcm();
    if (spCcm)
    {
        const std::string ccm_name = "Color Correction Matrix";
        _spCcmControls = _spISPPipelineTab->AddHapiControl<CcmControls>(std::move(ccm_name), std::move(spCcm), _debugMode);
    }
}

void TabbedUiControls::CreateOutputConfigTab()
{
    auto outputConfigControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    _spOutputConfigTab = _appCreateUiTabCB("Output Config", outputConfigControlsEnabledCB, "Controls for configuring the output from the ISP pipeline.");

    auto spUsm = _spPipeline->GetUsm();
    if (spUsm)
    {
        _spUsmControls = _spOutputConfigTab->AddHapiControl<UsmControls>(spUsm, _debugMode);
    }

    //// HDR related

    auto spLut1dLinearSrgb = _spPipeline->GetLut1dLinearSrgb();

    if (spLut1dLinearSrgb)
    {
        static constexpr bool BYPASS_BY_DEFAULT = true;

        _sp1dLutLinearSrgbControls = _spOutputConfigTab->AddHapiControl<Lut1dControls>(spLut1dLinearSrgb, nullptr, nullptr, _debugMode, BYPASS_BY_DEFAULT, "1D LUT Linear->RGB");
        _sp1dLutLinearSrgbControls->SetSettingSectionName("LUT1DLinearRgb");

        static constexpr uint32_t LUT1D_AUTO_LOAD_INTERVAL_MS = 1000;

        std::size_t task_idx = _spTaskHandler->CreateTask([this](){
            _sp1dLutLinearSrgbControls->AutoUpdateCallback();
        }, LUT1D_AUTO_LOAD_INTERVAL_MS);

        auto lut1dToggleUpdateCB = [this, task_idx](bool enabled)
        {
            enabled ? _spTaskHandler->EnableTask(task_idx) : _spTaskHandler->DisableTask(task_idx);
        };

        _sp1dLutLinearSrgbControls->SetAutoUpdateToggleFunc(lut1dToggleUpdateCB);
    }

    auto sp3dLutSrgbHlg = _spPipeline->GetLut3dSrgbHlg();

    if (sp3dLutSrgbHlg)
    {
        _sp3dLutSrgbHlgControls = _spOutputConfigTab->AddHapiControl<Lut3dControls>(std::move(sp3dLutSrgbHlg), "3D LUT sRGB->HLG", "ThreeDLUTsRgbToHlg");
    }

    auto sp3dLut = _spPipeline->GetLut3d();

    if (sp3dLut)
    {
        _sp3dLutControls = _spOutputConfigTab->AddHapiControl<Lut3dControls>(std::move(sp3dLut));
    }

    // Start a new UI column
    if(_sp1dLutLinearSrgbControls || _sp3dLutControls)
        _spOutputConfigTab->AddColumnBreak();

    auto spTmoBase = _spPipeline->GetTmoBase();
    if (spTmoBase)
    {
        _spTmoControls = _spOutputConfigTab->AddHapiControl<TmoControls>(SwApi::ITmo::Create(spTmoBase));
    }

    auto coreIspPipeline = _spPipeline->GetCoreIspPipeline();
    auto spWarpAdapter = coreIspPipeline->GetWarpAdapter();

    if (spWarpAdapter)
    {
        auto warpEnableCB = [this](bool enable) {
            _spPipeline->WarpEnabled(0, enable);
        };

        CapturePictureCB warpCaptureCB{nullptr};

        if(_warpCaptureCB)
        {
            warpCaptureCB = [this]() {
                std::cout << "Capture" << std::flush << std::endl;
                _warpCaptureCB(0);
            };
        }

        const std::string warp_name = "Warp";
        _spWarpControls = _spOutputConfigTab->AddHapiControl<WarpFullControls>(warp_name, spWarpAdapter, true, warpCaptureCB, warpEnableCB);

        if(_spWarpControls)
            _spWarpControls->EnableDebugControls(_powerUser);
    }

    auto spLut1d = _spPipeline->GetLut1d();
    auto spHs = coreIspPipeline->GetHs();
    if (spLut1d)
    {
        static constexpr bool BYPASS_BY_DEFAULT = false;

        _sp1dLutControls = _spOutputConfigTab->AddHapiControl<Lut1dControls>(std::move(spLut1d), spHs, SwApi::ITmoOverride::Create(spTmoBase), _debugMode, BYPASS_BY_DEFAULT);

        static constexpr uint32_t LUT1D_AUTO_LOAD_INTERVAL_MS = 1000;

        std::size_t task_idx = _spTaskHandler->CreateTask([this](){
            _sp1dLutControls->AutoUpdateCallback();
        }, LUT1D_AUTO_LOAD_INTERVAL_MS);

        auto lut1dToggleUpdateCB = [this, task_idx](bool enabled)
        {
            enabled ? _spTaskHandler->EnableTask(task_idx) : _spTaskHandler->DisableTask(task_idx);
        };

        _sp1dLutControls->SetAutoUpdateToggleFunc(lut1dToggleUpdateCB);
    }

    // Start a new UI column
    _spOutputConfigTab->AddColumnBreak();

    // Logo and screensaver controls
    auto spOutputMixer = _spPipeline->GetOutputMixer();
    auto spLogoHelper = _spPipeline->GetLogoHelper();
    if (spOutputMixer)
    {
        // Initialise timeout handler, with a period of 5 mins
        static constexpr uint32_t SCREENSAVER_TIMEOUT = 300;

        auto onScreensaverCB = [this](bool active){
            if(_spLogoControls)
                _spLogoControls->OnScreensaver(active);

            if(_spPipeline)
                _spPipeline->OnScreensaver(active);
        };

        _spScreensaver = std::make_shared<Screensaver>(SCREENSAVER_TIMEOUT, onScreensaverCB, VvpIspDemo::Get()->GetActivityNotifier());

        _spLogoControls = _spOutputConfigTab->AddHapiControl<LogoControls>(std::move(spOutputMixer), spLogoHelper, _debugMode);

        auto enableScreensaverCB = [this](bool enable) -> void {
            _spScreensaver->Enable(enable);
        };

        _spLogoControls->SetEnableScreensaverCB(enableScreensaverCB);
    }

    auto vfwCaptureRaw = _spPipeline->GetVfwCaptureRaw();
    auto vfwCaptureIsp = _spPipeline->GetVfwCaptureIsp();
    
    if(vfwCaptureRaw || vfwCaptureIsp)
    {
        _spVfwControls = _spOutputConfigTab->AddHapiControl<VvpIspVfwControls>(_spPipeline);
    }

    std::vector<std::string> outputSourceOpts{};

    outputSourceOpts.emplace_back("ISP");
    outputSourceOpts.emplace_back("Color Bars");

    auto outputSourceCb = [this](const uint32_t idx){

        // When mixer TPG used to generate Color bars
        // 1D LUT is put into bypass mode
        // and the UI controls are disabled to prevent
        // user from changing any settings

        bool enable1DLUTControls = (idx == 0);
        _sp1dLutControls->Enable(enable1DLUTControls);

        _spPipeline->SelectOutput(idx);
    };

    std::vector<std::string> vsOverrideOpts{};

    vsOverrideOpts.emplace_back("Not set");

    auto vsOverrideCb = [this](const uint32_t idx, const bool bpc10){
        _spPipeline->RequestOutputVideoStandardOverride(idx, bpc10);
    };

    _spOutputControls = _spOutputConfigTab->AddHapiControl<OutputControls>("Output", outputSourceOpts, outputSourceCb, vsOverrideOpts, vsOverrideCb, _powerUser);
}


void TabbedUiControls::UpdateOutputParams(const ImageConfig& outputConfig)
{
    if (!_uiInitialised)
        return;

    if (_spWarpControls)
        _spWarpControls->UpdateOutputResolution(outputConfig._width, outputConfig._height);
}

void TabbedUiControls::UpdateMixerParams(const ImageConfig& mixerConfig)
{
    if (!_uiInitialised)
        return;

}


void TabbedUiControls::CreateStatsTab()
{
    auto statsTabControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    _spStatisticsTab = _appCreateUiTabCB("Pipeline Statistics", statsTabControlsEnabledCB, "Controls for viewing the statistics from the ISP pipeline.");


    auto coreIspPipeline = _spPipeline->GetCoreIspPipeline();

    auto spBls = coreIspPipeline->GetBls();
    if(spBls)
    {
        _spBlsControls = _spStatisticsTab->AddHapiControl<BlsControls>(std::move(spBls), _debugMode);
    }

    auto spWbs = coreIspPipeline->GetWbs();
    if(spWbs)
    {
        const std::string wbs_name = "White Blanace Statistics";
        _spWbsControls = _spStatisticsTab->AddHapiControl<WbsControls>(wbs_name, std::move(spWbs), _debugMode, _powerUser, false);
    }


    auto wbsSwitchRouterToggle = [this, coreIspPipeline](bool enabled)
    {
        auto router = coreIspPipeline->GetSwitchRouter();
        if (router)
        {
            if (enabled)
            {
                router->SetWBSInput(SwitchRouterInput::PostWBC);
            }
            else
            {
                router->SetWBSInput(SwitchRouterInput::PreWBC);
            }
        }
    };
    _spWbsControls->LoadSwitchRouterFunc(wbsSwitchRouterToggle);


    auto spHs = coreIspPipeline->GetHs();

    if (spHs)
    {
        const std::string hs_name = "Histogram Statistics";
        auto roi = coreIspPipeline->GetROI();
        _spHsControls = _spStatisticsTab->AddHapiControl<HsControls>(hs_name, std::move(spHs), roi, _debugMode);

        static constexpr uint32_t HS_AUTOLOAD_INTERVAL_MS = 1000;

        std::size_t task_idx = _spTaskHandler->CreateTask([this](){
            _spHsControls->AutoUpdateCallback();
        }, HS_AUTOLOAD_INTERVAL_MS);

        auto hsToggleUpdateCB = [this, task_idx](bool enabled)
        {
            enabled ? _spTaskHandler->EnableTask(task_idx) : _spTaskHandler->DisableTask(task_idx);
        };

        _spHsControls->SetAutoUpdateToggleFunc(hsToggleUpdateCB);
    }
}

void TabbedUiControls::CreateCalibrationTab()
{
    auto coreIspPipelineCalibration = _spPipeline->GetCoreIspPipeline(0);
    _spTmoOverride = SwApi::ITmoOverride::Create(_spPipeline->GetTmoBase());

    auto calibrationTabControlsEnabledCB = [this, coreIspPipelineCalibration](uint32_t clientID, bool enabled)
    {
        if (enabled)
        {
            // FIXME - maybe integrate this into AWB/AEX rather than working like this?
            auto spCamera = coreIspPipelineCalibration->GetActiveCamera();

            if(spCamera)
                _backedUpCameraGain = spCamera->GetAnalogueGain();

            static const unsigned int highlightThreshold = 9000;
            static const unsigned int highlightVolume = 100;

            if(!_spTmoOverride->RequestOverride())
            {
                std::cerr << "Failed to request TMO override for calibration tab.\n";
                return;
            }

            _spTmoOverride->SetBypass(false);
            _spTmoOverride->SetEnableRoi(true);
            _spTmoOverride->SetRoiOutside(false);
            _spTmoOverride->SetThreshold(highlightThreshold);
            _spTmoOverride->SetLevel(highlightVolume);

            _spProfileOverviewControls->UpdateUiWithCurrentProfile();
            if(_spProfileBlackLevelControls)
            {
                _spProfileBlackLevelControls->UpdateUiWithCurrentProfile();
            }
            _spProfileWhiteBalanceControls->UpdateUiWithCurrentProfile();
            _spProfileRegionSelectControls->UpdateAllCoreRoI();

            _spTaskHandler->DisableTask(_awbTaskHandle);

            // Disable auto exposure if running
            if(_spAutoExposureControls)
            {
                _spAutoExposureControls->Enable(false);
                _autoExpEnableState = _spAutoExposureControls->AeEnabled();

                if(_autoExpEnableState)
                    _spTaskHandler->DisableTask(_aeTaskHandle);
            }

            auto warp = coreIspPipelineCalibration->GetWarpAdapter();
            if (warp)
            {
                auto configurator = warp->GetConfiguratorSafe();

                _warpHFlip = configurator->GetHMirror();
                _warpVFlip = configurator->GetVMirror();

                configurator->SetHMirror(false);
                configurator->SetVMirror(false);
            }

            auto ccm = coreIspPipelineCalibration->GetCcm();
            ccm->ApplySingularMatrix(vvp::ccm::GenerateIdentityMatrix());
        }
        else
        {
            _spTmoOverride->ReleaseOverride();

            _spTaskHandler->EnableTask(_awbTaskHandle);

            // Re-enable autoexposure
            if(_autoExpEnableState)
                _spTaskHandler->EnableTask(_aeTaskHandle);

            _spAutoExposureControls->Enable(true);

            auto spCamera = coreIspPipelineCalibration->GetActiveCamera();

            if(spCamera)
                spCamera->SetAnalogueGain(_backedUpCameraGain);

            auto warp = coreIspPipelineCalibration->GetWarpAdapter();
            if (warp)
            {
                auto configurator = warp->GetConfiguratorSafe();
                configurator->SetHMirror(_warpHFlip);
                configurator->SetVMirror(_warpVFlip);
            }

            auto ccm = coreIspPipelineCalibration->GetCcm();
            ccm->ApplyStoredMatrices();
        }
    };

    _spCalibrationTab = _appCreateUiTabCB("Sensor Calibration", calibrationTabControlsEnabledCB, "Sensor calibration controls.");

    auto wbProfile = coreIspPipelineCalibration->GetProfile();
    auto wbController = coreIspPipelineCalibration->GetWhiteBalanceController();

    _spProfileOverviewControls = _spCalibrationTab->AddHapiControl<VvpIspProfileOverviewControls>(wbProfile);
    _spProfileRegionSelectControls = _spCalibrationTab->AddHapiControl<VvpIspProfileMasterRegionControls>(
            _spTmoOverride,
            coreIspPipelineCalibration->GetHs(),
            coreIspPipelineCalibration->GetBls(),
            coreIspPipelineCalibration->GetWbs()
            );

    if(coreIspPipelineCalibration->GetBls())
    {
        _spProfileBlackLevelControls = _spCalibrationTab->AddHapiControl<VvpIspProfileBlackLevelControls>(
            _spPipeline,
            wbProfile,
            wbController,
            coreIspPipelineCalibration->GetActiveCamera(),
            coreIspPipelineCalibration->GetBls(),
            coreIspPipelineCalibration->GetAnr()
            );
    }

    _spProfileVignetteCorrectionControls = _spCalibrationTab->AddHapiControl<VvpIspProfileVignetteCorrectionControls>(
            wbProfile,
            wbController,
            coreIspPipelineCalibration->GetVc(),
            coreIspPipelineCalibration->GetHs()
        );
    _spProfileWhiteBalanceControls = _spCalibrationTab->AddHapiControl<VvpIspProfileWhiteBalanceControls>(
            wbProfile,
            wbController,
            coreIspPipelineCalibration->GetWbc(),
            coreIspPipelineCalibration->GetCcm()
            );
    /*
    _spProfileColourCorrectionControls = _spCalibrationTab->AddHapiControl<VvpIspProfileColourCorrectionControls>(
            wbProfile,
            wbController,
            coreIspPipelineCalibration->GetCcm()
            );
    */

    auto updateAllUiControls = [this](std::string title)
    {
        _spProfileOverviewControls->UpdateUiWithCurrentProfile();
        if(_spProfileBlackLevelControls)
        {
            _spProfileBlackLevelControls->UpdateUiWithCurrentProfile();
        }
        //_spProfileVignetteCorrectionControls->UpdateUiWithCurrentProfile();
        _spProfileWhiteBalanceControls->UpdateUiWithCurrentProfile();
        //_spProfileColourCorrectionControls->UpdateUiWithCurrentProfile();

        _spCameraProfileControls->ProfileLoadedExternally(title);
    };
    _spProfileOverviewControls->AddProfileLoadUiUpdateCallback(updateAllUiControls);

    auto updateProfileOverviewControl = [this](void)
    {
        _spProfileOverviewControls->UpdateUiWithCurrentProfile();
    };
    if(_spProfileBlackLevelControls)
    {
        _spProfileBlackLevelControls->LoadFunctionPointerForUpdatingOverviewUI(updateProfileOverviewControl);
    }
    _spProfileWhiteBalanceControls->LoadFunctionPointerForUpdatingOverviewUI(updateProfileOverviewControl);
    _spProfileVignetteCorrectionControls->LoadFunctionPointerForUpdatingOverviewUI(updateProfileOverviewControl);
}

void TabbedUiControls::CreateDebugTab()
{
    auto debugTabControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    _spDebugTab = _appCreateUiTabCB("==DEBUG==", debugTabControlsEnabledCB, "Debugging controls. Should not be active in normal circumstances.");

    auto spI2C = SwApi::I2C::Create();
    _spI2CControls = _spDebugTab->AddHapiControl<I2CControls>(std::move(spI2C));
}


void TabbedUiControls::PostTabCreationUpdates()
{
    if (!_uiInitialised)
        return;

    // Make sure only one camera control is active at startup
    for(const auto& c : _cameraControls)
    {
        c->SetActive(_spActiveCameraUiControls == c);

        // Temporary workaround for the RAW capture lockup
        c->EnableHdr(true);
        c->EnableHdr(false);
    }

    std::filesystem::path fileImportDestination = std::filesystem::current_path();
    fileImportDestination /= "awb_profile.json"; // Yuck for hardcoding, but needed for the demo

    // Make sure BLC and WBC controls are in correct state after profile load
    if(_spCameraProfileControls)
        _spCameraProfileControls->LoadProfile("Default", std::move(fileImportDestination));
    if(_spBlcControls)
        _spBlcControls->UpdateControlsFromHardware();
    if(_spWbcControls)
        _spWbcControls->UpdateControlsFromHardware();

    // Force apply persisted video input after the pipeline has been initialised
    _spSourceSelector->ApplySelectedInput();
}

void TabbedUiControls::UpdateUiPostOutputChange(ImageConfig& config)
{
    if (!_uiInitialised)
        return;

    if (_spTpgControls)
    {
        _spTpgControls->FollowOutputResCallback(config._width, config._height,
                                                (_spPipeline->GetUiSelectedInput() == PipelineInput::TPG));

    }
}

void TabbedUiControls::UpdateBlackLevel(const std::array<uint32_t, 4>& blc)
{
    for(auto cam: _cameraControls)
        cam->UpdateHdrBlackLevel(blc);
}


void TabbedUiControls::UpdateOutputStatus(const uint32_t status, const std::string& format_str, const std::vector<uint32_t>& format_opts, const bool bpc10_support)
{
    _spOutputControls->UpdateStatus(status, format_str, format_opts, bpc10_support);
}


void TabbedUiControls::UpdateInputFrameReaderControls(const SwApi::VfrSourceMetadata& metadata)
{
    if (_spFrameReaderControls)
        _spFrameReaderControls->UpdateSourceMetadata(metadata);
}
