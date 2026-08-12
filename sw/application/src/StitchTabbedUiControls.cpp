/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "CommonApplicationBase.h"
#include "StitchTabbedUiControls.h"

#include <ranges>
#include "CoeffGen.h"
#include "VvpIspDemo.h"



StitchTabbedUiControls::StitchTabbedUiControls(std::function<std::shared_ptr<TopLevelUiTab>(const std::string&, BooleanControlCB, const std::string&)> appCreateUiTabCB,
                                    std::shared_ptr<UdxVvpIspPipeline> spPipeline, std::shared_ptr<StitchPipeline> spStitchPipeline, bool debugMode, bool powerUser, std::function<void(uint32_t)> warpCaptureCB)
    : _appCreateUiTabCB(appCreateUiTabCB),
      _spTaskHandler{std::make_shared<TaskHandler>()},
      _spPipeline(spPipeline),
      _spStitchPipeline(spStitchPipeline),
      _debugMode(debugMode),
      _powerUser(powerUser),
      _warpCaptureCB(warpCaptureCB),
      _spActiveCameraUiControls(nullptr, nullptr),
      _autoExpEnableState{false},
      _aeTaskHandle{std::numeric_limits<std::size_t>::max()},
      _awbTaskHandle{std::numeric_limits<std::size_t>::max()}
{
    _uiInitialised = false;
}

StitchTabbedUiControls::~StitchTabbedUiControls()
{
    for (size_t pipeline_index = 0; pipeline_index < _histogramPreviewSubscriptionIds.size(); ++pipeline_index)
    {
        const auto subscriptionId = _histogramPreviewSubscriptionIds[pipeline_index];
        if (subscriptionId == 0)
            continue;

        auto spHs = _wpHistogramPreviewSources[pipeline_index].lock();
        if (spHs)
        {
            spHs->UnregisterHistogramObserver(subscriptionId);
        }
    }
}

void StitchTabbedUiControls::EnableUIUpdates()
{
    _spTaskHandler->Start();
    _uiInitialised = true;
}


std::shared_ptr<VvpIspSourceSelectControls> StitchTabbedUiControls::CreateInputSelector()
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

            uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
            for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
            {
                switch(selected_input)
                {
                    case PipelineInput::Cameras:
                        mipi_index = pipeline_index;
                    break;
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

                if(_ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls)
                {
                    _spTaskHandler->DisableTask(_awbTaskHandle[pipeline_index]);
                }

                if(_ispPipelineInputControls[pipeline_index]._spAutoExposureControls)
                {
                    _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->Enable(false);
                    aeEnabled = _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->AeEnabled();

                    if(aeEnabled)
                        _spTaskHandler->DisableTask(_aeTaskHandle[pipeline_index]);
                }

                if(mipi_index < numCams)
                {
                    // Set active camera controls
                    if(_spActiveCameraUiControls[mipi_index])
                        _spActiveCameraUiControls[mipi_index]->SetActive(false);

                    _spActiveCameraUiControls[mipi_index] = (mipi_index < _cameraControls.size()) ? _cameraControls[mipi_index] : nullptr;
                }
                
                auto coreIspPipeline = _spPipeline->GetCoreIspPipeline(pipeline_index);
                // This will update AutoExposure and WhiteBalance controllers
                coreIspPipeline->SetActiveCamera((mipi_index < numCams) ? cameras[mipi_index] : nullptr);

                if(mipi_index < numCams)
                {
                    if(_spActiveCameraUiControls[mipi_index])
                        _spActiveCameraUiControls[mipi_index]->SetActive(true);
                }

                // Update AWB Black Level after changing cameras
                // If Auto Exposure is enabled it will be overwritten
                if(_ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls && _ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls->IsAwbEnabled())
                {
                    coreIspPipeline->GetWhiteBalanceController()->ApplyBlackLevel();
                }

                // Re-enable handlers after the camera has been changed
                if (_ispPipelineInputControls[pipeline_index]._spAutoExposureControls)
                {
                    const bool enableAutoExposureControls = (pipeline_index < numCams);

                    if(enableAutoExposureControls)
                    {
                        if(aeEnabled)
                            _spTaskHandler->EnableTask(_aeTaskHandle[pipeline_index]);
                    }

                    _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->Enable(enableAutoExposureControls);
                }
                if(_ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls)
                {
                    _spTaskHandler->EnableTask(_awbTaskHandle[pipeline_index]);
                }
            }
        }
    };

    return _spInputConfigTab->AddHapiControl<VvpIspSourceSelectControls>("Input Source", std::move(inputOpts), inputSelectorCb);
}


void StitchTabbedUiControls::CreateInputConfigTab()
{
    auto inputConfigControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
        if (enabled)
        {
            // Do nothing
        }
        else
        {
            uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
            for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
            {
                // Disable ROI highlighting and release TMO override
                _ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls->DisableRoiHighlighting();
                _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->DisableRoiHighlighting();
            }
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
                auto pipelines = _spPipeline->GetNumberCoreIspPipelines();
                for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
                {
                    auto coreIspPipeline = _spPipeline->GetCoreIspPipeline(pipeline_index);
                    auto activeCamera = coreIspPipeline->GetActiveCamera();
                    if(activeCamera)
                    {
                        if(mipi_index == activeCamera->GetMIPIInterface())
                        {
                            coreIspPipeline->GetWhiteBalanceController()->SetGain(v);
                        }
                    }

                    if(_ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls->IsAwbEnabled())
                    {
                        coreIspPipeline->GetWhiteBalanceController()->ApplyBlackLevel();
                    }
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

    auto spMultiChannelAutoExposure = _spStitchPipeline->GetMultiChannelAutoExposure();
    if(spMultiChannelAutoExposure)
    {
        _spMultiChannelAutoExposureControls = _spInputConfigTab->AddHapiControl<MultiChannelAutoExposureControls>(spMultiChannelAutoExposure);
    }

    auto spMultiChannelWhiteBalance = _spStitchPipeline->GetMultiChannelWhiteBalance();
    if(spMultiChannelWhiteBalance)
    {
        _spMultiChannelWhiteBalanceControls = _spInputConfigTab->AddHapiControl<MultiChannelWhiteBalanceControls>(spMultiChannelWhiteBalance);
    }

    auto ispInputControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    auto pipelines = _spPipeline->GetNumberCoreIspPipelines();
    _ispPipelineInputControls.resize(pipelines);
    for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
    {
        const std::string isp_input_tab_name = "ISP Input Pipeline " + std::to_string(pipeline_index);
        _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab = _appCreateUiTabCB(isp_input_tab_name, ispInputControlsEnabledCB, "Controls for configuring the ISP pipeline cores.");

        auto coreIspPipeline = _spPipeline->GetCoreIspPipeline(pipeline_index);

        const std::string profile_loader_name = "Sensor Profile " + std::to_string(pipeline_index);
        _ispPipelineInputControls[pipeline_index]._spCameraProfileControls = _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddHapiControl<VvpIspProfileLoaderControls>(
            profile_loader_name,
            coreIspPipeline->GetProfile(),
            coreIspPipeline->GetWhiteBalanceController()
        );

        // Histogram preview panel
        auto spHs = coreIspPipeline->GetHs();
        if (spHs)
        {
            const std::string histogram_name = "Histogram " + std::to_string(pipeline_index);
            _ispPipelineInputControls[pipeline_index]._spHistogramPreview = _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddHapiControl<HistogramPreview>(histogram_name);

            _wpHistogramPreviewSources[pipeline_index] = spHs;
            _histogramPreviewSubscriptionIds[pipeline_index] = spHs->RegisterHistogramObserver([this, pipeline_index](const std::shared_ptr<SwApi::Hs::TableResults>& results){
                if (_ispPipelineInputControls[pipeline_index]._spHistogramPreview && results)
                {
                    _ispPipelineInputControls[pipeline_index]._spHistogramPreview->Update(*results);
                }
            });
        }


        // Clipper
        if(_debugMode || _powerUser)
        {
            auto spClipper = coreIspPipeline->GetClipper();

            if (spClipper)
            {
                _ispPipelineInputControls[pipeline_index]._spClipperControls = _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddHapiControl<ClipperControls>(spClipper, true);

                auto clipperUpdatePipelineCB = [this, pipeline_index] (void)
                {
                    _spPipeline->ReprogramPipeAction(UdxVvpIspPipeline::PipelineStateChanged::PreWarpNoClipperReset);
                };
                _ispPipelineInputControls[pipeline_index]._spClipperControls->LoadUpdatePipelineCB(clipperUpdatePipelineCB);

                static constexpr uint32_t CLIPPER_INTERVAL_MS = 100;

                std::size_t clipper_task_idx = _spTaskHandler->CreateTask([this, pipeline_index](){
                    _ispPipelineInputControls[pipeline_index]._spClipperControls->PolledUpdateCB();
                }, CLIPPER_INTERVAL_MS);

                _spTaskHandler->EnableTask(clipper_task_idx);
            }
        }

        _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddColumnBreak();

        // AutoExposure
        auto aeController = coreIspPipeline->GetAutoExposureController();

        if (aeController)
        {
            auto roi = coreIspPipeline->GetROI();

            const std::string auto_exposure_name = "Auto Exposure " + std::to_string(pipeline_index);
            _ispPipelineInputControls[pipeline_index]._spAutoExposureControls = _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddHapiControl<VvpIspAutoExposureControls>(auto_exposure_name, coreIspPipeline->GetAutoExposureController(), roi, _debugMode, _powerUser);

            static constexpr uint32_t AUTOEXPOSURE_INTERVAL_MS = 100;

            std::size_t task_idx = _spTaskHandler->CreateTask([this, coreIspPipeline, pipeline_index](){
                auto activeCamera = coreIspPipeline->GetActiveCamera();
                uint32_t mipi_index = activeCamera ? activeCamera->GetMIPIInterface() : 0xFFFFFFFF;

                _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->AeTask();

                if(mipi_index < sizeof(_spActiveCameraUiControls)/sizeof(CameraUiControlsPtr))
                {
                    auto aeController = coreIspPipeline->GetAutoExposureController();

                    if(aeController)
                    {
                        const float shutterSpeed = aeController->GetShutterSpeed();
                        const float analogueGain = aeController->GetAnalogueGain();
                        const float digitalGain = aeController->GetDigitalGain();

                        if (_spActiveCameraUiControls[mipi_index])
                        {
                            auto values_nearly_same = [](const float v1, const float v2)->bool {
                                static constexpr float EPSILON = 0.0000001f;
                                return std::fabs(v1 - v2) < EPSILON;
                            };

                            if(!values_nearly_same(shutterSpeed, _spActiveCameraUiControls[mipi_index]->GetShutterSpeed())){
                                _spActiveCameraUiControls[mipi_index]->SetShutterSpeed(shutterSpeed);
                            }

                            if(!values_nearly_same(analogueGain, _spActiveCameraUiControls[mipi_index]->GetAnalogueGain())){
                                _spActiveCameraUiControls[mipi_index]->SetAnalogueGain(analogueGain);
                            }

                            if(!values_nearly_same(digitalGain, _spActiveCameraUiControls[mipi_index]->GetDigitalGain())){
                                _spActiveCameraUiControls[mipi_index]->SetDigitalGain(digitalGain);
                            }
                        }
                    }
                }
            }, AUTOEXPOSURE_INTERVAL_MS);

            _aeTaskHandle[pipeline_index] = task_idx;

            auto autoExposureEnableCB0 = [this, task_idx](bool enable){
                enable ? _spTaskHandler->EnableTask(task_idx) : _spTaskHandler->DisableTask(task_idx);
            };

            _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->SetAutoUpdateToggleFunc(autoExposureEnableCB0);
        }

        _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddColumnBreak();

        auto roi = coreIspPipeline->GetROI();

        const std::string auto_white_balance_name = "Auto White Balance " + std::to_string(pipeline_index);
        _ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls = _ispPipelineInputControls[pipeline_index]._spISPPipelineInputTab->AddHapiControl<VvpIspAutoWhiteBalanceControls>(
            auto_white_balance_name,
            coreIspPipeline->GetWhiteBalanceController(),
            roi
        );

        static constexpr uint32_t AUTO_WHITE_BALANCE_INTERVAL_MS = 100;

        std::size_t awb_task_idx = _spTaskHandler->CreateTask([this, pipeline_index](){
            _ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls->AutoWhiteBalanceUpdateFunc();
        }, AUTO_WHITE_BALANCE_INTERVAL_MS);

        
        _awbTaskHandle[pipeline_index] = awb_task_idx;

        _spTaskHandler->EnableTask(awb_task_idx);

        auto awbUpdateBLCAndWBCCB = [this, &coreIspPipeline, pipeline_index](bool override)
        {
            if (_ispPipelineControls[pipeline_index]._spBlcControls)
            {
                _ispPipelineControls[pipeline_index]._spBlcControls->BlockUIBasedOnBLSCallback(override);
            }
            if (_ispPipelineControls[pipeline_index]._spWbcControls)
            {
                _ispPipelineControls[pipeline_index]._spWbcControls->BlockUIBasedOnAWBCallback(override);
            }
            if (_ispStatisticsControls[pipeline_index]._spBlsControls)
            {
                auto spBlc = coreIspPipeline->GetBlc();
                auto pedestals = spBlc->GetBlackPedestals();
                _ispStatisticsControls[pipeline_index]._spBlsControls->UpdateGraphValues(pedestals[0], pedestals[1], pedestals[2], pedestals[3],
                                                (pedestals[0] + pedestals[1] + pedestals[2] + pedestals[3]) / 4);
            }
        };

        _ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls->LoadBlockBlcWbcFunctionPointer(awbUpdateBLCAndWBCCB);

        auto awbSetBypassBLCAndWBCCB = [this, pipeline_index](bool bypass)
        {
            if (_ispPipelineControls[pipeline_index]._spBlcControls)
            {
                _ispPipelineControls[pipeline_index]._spBlcControls->BypassBasedOnAWBCallback(bypass);
            }
            if (_ispPipelineControls[pipeline_index]._spWbcControls)
            {
                _ispPipelineControls[pipeline_index]._spWbcControls->BypassBasedOnAWBCallback(bypass);
            }
        };
        _ispPipelineInputControls[pipeline_index]._spAutoWhiteBalanceControls->SetBypassBLCAndWBCFunctionPointer(awbSetBypassBLCAndWBCCB);
    }
}

void StitchTabbedUiControls::UpdateInputParams(const ImageConfig& inputConfig)
{
    if (!_uiInitialised)
        return;
}

void StitchTabbedUiControls::CreateISPTab()
{
    auto ispControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();

    _ispPipelineControls.resize(pipelines);

    for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
    {
        const std::string isp_tab_name = "ISP Pipeline " + std::to_string(pipeline_index);
        _ispPipelineControls[pipeline_index]._spISPPipelineTab = _appCreateUiTabCB(isp_tab_name, ispControlsEnabledCB, "Controls for configuring the ISP pipeline cores.");

        auto coreIspPipeline = _spPipeline->GetCoreIspPipeline(pipeline_index);
        auto spDpc = coreIspPipeline->GetDpc();
        if (spDpc)
        {
            const std::string dpc_name = "Defective Pixel Correction " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spDpcControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<DpcControls>(std::move(dpc_name), std::move(spDpc), _debugMode);
        }

        auto spAnr = coreIspPipeline->GetAnr();
        if (spAnr)
        {
            const std::string anr_name = "Adaptive Noise Reduction " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spAnrControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<AnrControls>(std::move(anr_name), std::move(spAnr));
        }

        auto spBlc = coreIspPipeline->GetBlc();
        if (spBlc)
        {
            const std::string blc_name = "Black Level Correction " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spBlcControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<BlcControls>(std::move(blc_name), std::move(spBlc), _debugMode, _powerUser);
        }

        auto spVc = coreIspPipeline->GetVc();
        if (spVc)
        {
            const std::string vc_name = "Vignette Correction " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spVcControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<VcControls>(std::move(vc_name), std::move(spVc), _debugMode);
        }

        auto spWbc = coreIspPipeline->GetWbc();
        if (spWbc)
        {
            const std::string wbc_name = "White Balance Correction " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spWbcControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<WbcControls>(std::move(wbc_name), std::move(spWbc), _debugMode);
        }

        auto spDemosaic = coreIspPipeline->GetDemosaic();
        if (spDemosaic)
        {
            const std::string demosaic_name = "Demosaic " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spDemosaicControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<DemosaicControls>(std::move(demosaic_name), std::move(spDemosaic), _debugMode);
        }

        auto spCcm = coreIspPipeline->GetCcm();
        if (spCcm)
        {
            const std::string ccm_name = "Color Correction Matrix " + std::to_string(pipeline_index);
            _ispPipelineControls[pipeline_index]._spCcmControls = _ispPipelineControls[pipeline_index]._spISPPipelineTab->AddHapiControl<CcmControls>(std::move(ccm_name), std::move(spCcm), _debugMode);
        }
    }
}

void StitchTabbedUiControls::CreateOutputConfigTab()
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

        _sp1dLutLinearSrgbControls = _spOutputConfigTab->AddHapiControl<Lut1dControls>(std::move(spLut1dLinearSrgb), nullptr, nullptr, _debugMode, BYPASS_BY_DEFAULT, "1D LUT Linear->RGB");
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

    auto coreIspPipeline0 = _spPipeline->GetCoreIspPipeline(0);
    uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
    if(pipelines == 1)
    {
        _vspWarpControls.resize(1);
        auto spWarpAdapter = coreIspPipeline0->GetWarpAdapter();

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
            _vspWarpControls[0] = _spOutputConfigTab->AddHapiControl<WarpFullControls>(warp_name, spWarpAdapter, true, warpCaptureCB, warpEnableCB, WarpFullControls::WarpControlMode::Fixed);

            if(_vspWarpControls[0])
                _vspWarpControls[0]->EnableDebugControls(_powerUser);
        }
    }

    auto spLut1d = _spPipeline->GetLut1d();
    auto spHs0 = coreIspPipeline0->GetHs();
    if (spLut1d)
    {
        static constexpr bool BYPASS_BY_DEFAULT = false;

        _sp1dLutControls = _spOutputConfigTab->AddHapiControl<Lut1dControls>(std::move(spLut1d), spHs0, SwApi::ITmoOverride::Create(spTmoBase), _debugMode, BYPASS_BY_DEFAULT);

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

        _spLogoControls = _spOutputConfigTab->AddHapiControl<LogoControls>(spOutputMixer, spLogoHelper, _debugMode);

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


void StitchTabbedUiControls::UpdateOutputParams(const ImageConfig& outputConfig)
{
    if (!_uiInitialised)
        return;

/*    for(auto& spWarpControls : _vspWarpControls)
    {
        if (spWarpControls)
            spWarpControls->UpdateOutputResolution(outputConfig._width, outputConfig._height);
    }*/
}

void StitchTabbedUiControls::UpdateMixerParams(const ImageConfig& mixerConfig)
{
    if (!_uiInitialised)
        return;

    for(auto& spWarpControls : _vspWarpControls)
    {
        if (spWarpControls)
            spWarpControls->UpdateOutputResolution(mixerConfig._width, mixerConfig._height);
    }

    if(_spStitchControls)
    {
        _spStitchControls->Update();
    }
}

void StitchTabbedUiControls::CreateStitchTab()
{
    auto stitchTabControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    _spStitchTab = _appCreateUiTabCB("Pipeline Stitch", stitchTabControlsEnabledCB, "Controls stitch from the ISP pipeline.");

    auto spStitchAdapter = _spStitchPipeline->GetStitchAdapter();
    if(spStitchAdapter)
    {
        _spStitchControls = _spStitchTab->AddHapiControl<StitchControls>(spStitchAdapter, _powerUser);
    }

    uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
    if(pipelines > 1)
    {
        _vspWarpControls.resize(pipelines);
        for(uint32_t pipeline_index = 0; pipeline_index < pipelines; pipeline_index++)
        {
            auto coreIspPipeline = _spPipeline->GetCoreIspPipeline(pipeline_index);
            auto spWarpAdapter = coreIspPipeline->GetWarpAdapter();

            if (spWarpAdapter)
            {
                auto warpEnableCB = [this, pipeline_index](bool enable) {
                    _spPipeline->WarpEnabled(pipeline_index, enable);
                };

                auto warpCaptureCB = [this, pipeline_index]() {
                    std::cout << "Capture" << std::flush << std::endl;
                    _warpCaptureCB(pipeline_index);
                };
                
                const std::string warp_name = "Warp " + std::to_string(pipeline_index);
                _vspWarpControls[pipeline_index] = _spStitchTab->AddHapiControl<WarpFullControls>(warp_name, spWarpAdapter, true, (pipeline_index == 0) ? std::function<void()>(warpCaptureCB) :  nullptr, warpEnableCB, WarpFullControls::WarpControlMode::Arbitrary);

                if(_vspWarpControls[pipeline_index])
                    _vspWarpControls[pipeline_index]->EnableDebugControls(_powerUser);
            }
        }
    }
}

void StitchTabbedUiControls::CreateStatsTab()
{
    auto statsTabControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
    _ispStatisticsControls.resize(pipelines);
    for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
    {
        const std::string statistics_tab_name = "Pipeline Statistics " + std::to_string(pipeline_index);
        _ispStatisticsControls[pipeline_index]._spStatisticsTab = _appCreateUiTabCB(statistics_tab_name, statsTabControlsEnabledCB, "Controls for viewing the statistics from the ISP pipeline.");

        auto coreIspPipeline = _spPipeline->GetCoreIspPipeline(pipeline_index);

        auto spBls = coreIspPipeline->GetBls();
        if(spBls)
        {
            _ispStatisticsControls[pipeline_index]._spBlsControls = _ispStatisticsControls[pipeline_index]._spStatisticsTab->AddHapiControl<BlsControls>(std::move(spBls), _debugMode);
        }

        auto spWbs = coreIspPipeline->GetWbs();
        const std::string wbs_name = "White Blanace Statistics " + std::to_string(pipeline_index);
        _ispStatisticsControls[pipeline_index]._spWbsControls = _ispStatisticsControls[pipeline_index]._spStatisticsTab->AddHapiControl<WbsControls>(wbs_name, std::move(spWbs), _debugMode, _powerUser, false);

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
        _ispStatisticsControls[pipeline_index]._spWbsControls->LoadSwitchRouterFunc(wbsSwitchRouterToggle);

        auto spHs = coreIspPipeline->GetHs();

        if (spHs)
        {
            const std::string hs_name = "Histogram Statistics " + std::to_string(pipeline_index);
            auto roi = coreIspPipeline->GetROI();
            _ispStatisticsControls[pipeline_index]._spHsControls = _ispStatisticsControls[pipeline_index]._spStatisticsTab->AddHapiControl<HsControls>(hs_name, std::move(spHs), roi, _debugMode);

            static constexpr uint32_t HS_AUTOLOAD_INTERVAL_MS = 1000;

            std::size_t task_idx = _spTaskHandler->CreateTask([this, pipeline_index](){
                _ispStatisticsControls[pipeline_index]._spHsControls->AutoUpdateCallback();
            }, HS_AUTOLOAD_INTERVAL_MS);

            auto hsToggleUpdateCB = [this, task_idx](bool enabled)
            {
                enabled ? _spTaskHandler->EnableTask(task_idx) : _spTaskHandler->DisableTask(task_idx);
            };

            _ispStatisticsControls[pipeline_index]._spHsControls->SetAutoUpdateToggleFunc(hsToggleUpdateCB);
        }
    }
}

void StitchTabbedUiControls::CreateCalibrationTab()
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

            uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
            for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
            {
                _spTaskHandler->DisableTask(_awbTaskHandle[pipeline_index]);

                // Disable auto exposure if running
                if(_ispPipelineInputControls[pipeline_index]._spAutoExposureControls)
                {
                    _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->Enable(false);
                    _autoExpEnableState = _ispPipelineInputControls[pipeline_index]._spAutoExposureControls->AeEnabled();

                    if(_autoExpEnableState)
                        _spTaskHandler->DisableTask(_aeTaskHandle[pipeline_index]);
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
        }
        else
        {
            _spTmoOverride->ReleaseOverride();

            _spTaskHandler->EnableTask(_awbTaskHandle[0]);

            // Re-enable autoexposure
            if(_autoExpEnableState)
                _spTaskHandler->EnableTask(_aeTaskHandle[0]);

            _ispPipelineInputControls[0]._spAutoExposureControls->Enable(true);

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

        uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
        for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
        {
            _ispPipelineInputControls[pipeline_index]._spCameraProfileControls->ProfileLoadedExternally(title);
        }
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

void StitchTabbedUiControls::CreateDebugTab()
{
    auto debugTabControlsEnabledCB = [this](uint32_t clientID, bool enabled)
    {
    };

    _spDebugTab = _appCreateUiTabCB("==DEBUG==", debugTabControlsEnabledCB, "Debugging controls. Should not be active in normal circumstances.");

    auto spI2C = SwApi::I2C::Create();
    _spI2CControls = _spDebugTab->AddHapiControl<I2CControls>(std::move(spI2C));
}


void StitchTabbedUiControls::PostTabCreationUpdates()
{
    if (!_uiInitialised)
        return;

    // Make sure only one camera control is active at startup
    for(const auto& c : _cameraControls)
    {
        c->SetActive(true);

        // Temporary workaround for the RAW capture lockup
        c->EnableHdr(true);
        c->EnableHdr(false);
    }

    std::filesystem::path fileImportDestination = std::filesystem::current_path();
    fileImportDestination /= "awb_profile.json"; // Yuck for hardcoding, but needed for the demo

    // Make sure BLC and WBC controls are in correct state after profile load
    uint32_t pipelines = _spPipeline->GetNumberCoreIspPipelines();
    for(uint32_t pipeline_index = 0U; pipeline_index < pipelines; pipeline_index++)
    {
        if(_ispPipelineInputControls[pipeline_index]._spCameraProfileControls)
            _ispPipelineInputControls[pipeline_index]._spCameraProfileControls->LoadProfile("Default", fileImportDestination);
        if(_ispPipelineControls[pipeline_index]._spBlcControls)
            _ispPipelineControls[pipeline_index]._spBlcControls->UpdateControlsFromHardware();
        if(_ispPipelineControls[pipeline_index]._spWbcControls)
            _ispPipelineControls[pipeline_index]._spWbcControls->UpdateControlsFromHardware();
    }

    // Force apply persisted video input after the pipeline has been initialised
    _spSourceSelector->ApplySelectedInput();
}

void StitchTabbedUiControls::UpdateUiPostOutputChange(ImageConfig& config)
{
    if (!_uiInitialised)
        return;

    if (_spTpgControls)
    {
        _spTpgControls->FollowOutputResCallback(config._width, config._height,
                                                (_spPipeline->GetUiSelectedInput() == PipelineInput::TPG));

    }
}

void StitchTabbedUiControls::UpdateBlackLevel(const std::array<uint32_t, 4>& blc)
{
    for(auto cam: _cameraControls)
        cam->UpdateHdrBlackLevel(blc);
}


void StitchTabbedUiControls::UpdateOutputStatus(const uint32_t status, const std::string& format_str, const std::vector<uint32_t>& format_opts, const bool bpc10_support)
{
    _spOutputControls->UpdateStatus(status, format_str, format_opts, bpc10_support);
}


void StitchTabbedUiControls::UpdateInputFrameReaderControls(const SwApi::VfrSourceMetadata& metadata)
{
    if (_spFrameReaderControls)
        _spFrameReaderControls->UpdateSourceMetadata(metadata);
}
