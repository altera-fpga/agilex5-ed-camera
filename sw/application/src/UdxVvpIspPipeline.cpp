/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "UdxVvpIspPipeline.h"
#include "CommandLineFlags.h"
#include <chrono>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mutex>
#include "ClipperUtils.h"
#include "VcMeshUtils.h"
#include "CompileTimeConfiguration.h"
#include "IspCommon.h"
#include "HapiPio.h"
#include "VideoStandard.h"
#include "Logging.h"
#include "intel_vvp_core.h"
#include "intel_vvp_csc.h"
#include "intel_vvp_snoop.h"
#include "SwitchUiStateImplementation.h"
#include "TpgUiStateImplementation.h"
#include "VvpIspDemo.h"
#include "CommonUiUpdate.h"
#include "dptx_formats.h"
#include "dptx_app_defs.h"
#include "HapiVvpThrottle.h"
#include "VideoThrottle.h"
#include "IUIConnection.h"

#ifdef CAMERA_FramosGMSL
#include "FramosGMSL.h"
#endif
#ifdef CAMERA_FramosIMX678
#include "FramosIMX678.h"
#endif
#ifdef CAMERA_SonyIMX477
#include "SonyIMX477.h"
#endif
#ifdef CAMERA_SonyIMX519
#include "SonyIMX519.h"
#endif

#include "helpers/IspVfrInput/IspVfrInput.h"

typedef struct _tCameraData
{
	const std::string node_id;
	std::shared_ptr<ICamera> (*create)(const uint32_t idx, const uint32_t targetFrameRate);
} tCameraData;

static tCameraData cameraList[] = {
#ifdef CAMERA_FramosIMX678
    {"framos-imx678", SwApi::FramosImx678::Create},
#endif
#ifdef CAMERA_SonyIMX477
    {"sony-imx477", SwApi::SonyImx477::Create},
#endif
#ifdef CAMERA_SonyIMX519
    {"sony-imx519", SwApi::SonyImx519::Create},
#endif
};


using namespace SwApi;
using namespace VideoPipeline;

#undef ALWAYS_REPROGRAM_PIPE


// Shorthand for UHD/HD resolutions
static constexpr VideoStandard ImageConfig2160p = VS_2160p60_RGB8();
static constexpr VideoStandard ImageConfig1080p = VS_1080p60_RGB8();


UdxVvpIspPipeline::UdxVvpIspPipeline():
    _spHapi(nullptr),
    _terminalCore{nullptr},
    _inputs{},
    _dpTxMultirateEnabled{false},
    _dpTxOverrideActive{false}
{
}

void UdxVvpIspPipeline::Initialize(std::shared_ptr<Hapi::IHapi> spHapi)
{
    _spHapi = spHapi;
    _spInputState = std::make_shared<PipelineInputState>();
    _spOutputState = std::make_shared<PipelineOutputState>();

    // Read FPGA build time and pipeline settings
    ReadSettingsPIO();

    bool vfwCaptureRawFound= InitVfwCaptureRaw();
    if(!vfwCaptureRawFound)
    {
        WARN << "Failed to initialise Raw capture Video Frame Writer.\n";
    }

    bool vfwCaptureIspFound= InitVfwCaptureIsp();
    if(!vfwCaptureIspFound)
    {
        WARN << "Failed to initialise ISP capture Video Frame Writer.\n";
    }     

    InitDpTxMultirate();

    bool inputTpgFound = InitTpg();

    if(inputTpgFound)
        _inputs.emplace_back(PipelineInput::TPG);
    else
        ERR << "Failed to initialise Tpg.\n";

    // Create Remosaic instance
    const uint32_t REMOSAIC_UID = PipelineBaseUID(PipelineSubsystem::Input);
    auto remosaic_hapi = _spHapi->CreateByUniqueID<Hapi::VvpRemosaic>(REMOSAIC_UID);

    if(remosaic_hapi)
        _spRemosaic = std::make_unique<SwApi::Remosaic>(remosaic_hapi);
    else
        WARN << "Remosaic not found.\n";

    bool camerasFound = InitCameras();

    if(camerasFound)
    {
#ifdef ISP_STITCH_BUILD
        _inputs.push_back(PipelineInput::Cameras);
#endif
        PipelineInput camera_inputs[] = {
            PipelineInput::Camera0,
            PipelineInput::Camera1,
            PipelineInput::Invalid
        };

        std::size_t camera_idx = 0;
        static constexpr std::size_t max_idx = sizeof(camera_inputs) / sizeof(std::underlying_type<PipelineInput>::type);

        // Start the cameras once initialised
        for(const auto& c: _cameras){
            c->Start();
            _inputs.push_back(camera_inputs[std::min(camera_idx, max_idx)]);
            ++camera_idx;
        }
    }
    else
    {
        ERR << "Video cameras not available!\n";
    }

    bool vfrInputFound = InitVfrInput();

    if(!vfrInputFound)
    {
        ERR << "Failed to initialise Frame Reader Input.\n";
    }

    // Check if input throttle is available
    const uint32_t TPG_INPUT_THROTTLE_UID = PipelineBaseUID(PipelineSubsystem::Input) + 0;

    auto tpgInputThrottle = _spHapi->CreateByUniqueID<Hapi::VvpThrottle>(TPG_INPUT_THROTTLE_UID);
    
    if(tpgInputThrottle)
    {
        altera_vvp_throttle_stop(tpgInputThrottle->GetInstance());
        altera_vvp_throttle_start(tpgInputThrottle->GetInstance());
        altera_vvp_throttle_set_clock_cycles_per_line(tpgInputThrottle->GetInstance(), 2200);
        altera_vvp_throttle_set_active_lines(tpgInputThrottle->GetInstance(), 2160);
        altera_vvp_throttle_set_v_blank_lines(tpgInputThrottle->GetInstance(), 90);
        altera_vvp_throttle_commit_writes(tpgInputThrottle->GetInstance());
    }

    // Check if input throttle is available
    const uint32_t ISP_PIPELINE_THROTTLE_UID = PipelineBaseUID(PipelineSubsystem::PostCoreISP) + 0;

    auto ispPipelineThrottle = _spHapi->CreateByUniqueID<Hapi::VvpThrottle>(ISP_PIPELINE_THROTTLE_UID);
    
    if(ispPipelineThrottle)
    {
        altera_vvp_throttle_stop(ispPipelineThrottle->GetInstance());
        altera_vvp_throttle_start(ispPipelineThrottle->GetInstance());
#ifdef MAX_30_FPS
        /* 30 fps output rate*/
        altera_vvp_throttle_set_clock_cycles_per_line(ispPipelineThrottle->GetInstance(), 4400);
#else
        /* 60 fps output rate*/
        altera_vvp_throttle_set_clock_cycles_per_line(ispPipelineThrottle->GetInstance(), 2200);
#endif
        altera_vvp_throttle_set_active_lines(ispPipelineThrottle->GetInstance(), 2160);
        altera_vvp_throttle_set_v_blank_lines(ispPipelineThrottle->GetInstance(), 90);
        altera_vvp_throttle_commit_writes(ispPipelineThrottle->GetInstance());
    }

    bool bayerSwitchFound = InitBayerSwitch();

    if (!bayerSwitchFound)
    {
        ERR << "Failed to initialise Bayer Switch.\n";
    }

    for(auto coreIspPipeline : _coreIspPipelines)
    {
        coreIspPipeline->InitCoreIspPipeline();
    }    

    if(false == InitHdrComponents())
        INFO << "HDR LUTs not available.\n";

    bool lut3dFound = InitLut3d();
    if (!lut3dFound)
    {
        ERR << "Failed to initialise 3D LUT.\n";
    }

    bool tmoFound = InitTmo();
    if (!tmoFound)
    {
        ERR << "Failed to initialise Tmo.\n";
    }

    bool usmFound = InitUsm();
    if (!usmFound)
    {
        ERR << "Failed to initialise USM.\n";
    }

    bool lut1dFound = InitLut1d();
    if (!lut1dFound)
    {
        ERR << "Failed to initialise 1D LUT.\n";
    }

    bool outputMixerTpgFound = InitOutputMixerTpg();
    if (!outputMixerTpgFound)
    {
        ERR << "Failed to initialise Output TPG Mixer.\n";
    }

    bool overlayFound = InitOverlayVfr();
    if (!overlayFound)
    {
        ERR << "Failed to initialise Overlay.\n";
    }

    bool outputMixerFound = InitOutputMixer();
    if (!outputMixerFound)
    {
        ERR << "Failed to initialise Output Mixer.\n";
    }

    bool outputPipConvFound = InitOutputPipConv();
    if (!outputPipConvFound)
    {
        ERR << "Failed to initialise output pip converter!\n";
    }

    if(_spOutputMixer)
    {
        _spOutputMixer->EnableIsp(true);
    }


    const uint32_t PROTO_CONV_LTF_DPTX_UID = PipelineBaseUID(PipelineSubsystem::Output);
    _spProtoConvLTFDpTx = _spHapi->CreateByUniqueID<Hapi::VvpProtocolConverter>(PROTO_CONV_LTF_DPTX_UID);

    if(_spProtoConvLTFDpTx)
    {
        auto pipelineOutputHandler = [this](const ImageConfig& outputDetails) -> ImageConfig {
            ASSERT(_protoConvLTFDpTxCore->_inputCores.size() == 1,
                    "The Protocol Converter requires a single input connected.");

            const std::lock_guard<std::recursive_mutex> lock(_mutex);

            if(_protoConvLTFDpTxCore->_currentOutputVideoDetails != outputDetails)
            {
                static constexpr uint8_t VVP_CHROMA_SAMPLING_Y444 = 3;
                static constexpr uint8_t VVP_COLOURSPACE_RGB = 0;
                
                auto instance = _spProtoConvLTFDpTx->GetInstance();
                intel_vvp_core_set_img_info_width(instance, outputDetails._width);
                intel_vvp_core_set_img_info_height(instance, outputDetails._height);
                intel_vvp_core_set_img_info_interlace(instance, 0);
                intel_vvp_core_set_img_info_colorspace(instance, VVP_COLOURSPACE_RGB);
                intel_vvp_core_set_img_info_subsampling(instance, VVP_CHROMA_SAMPLING_Y444);
                intel_vvp_core_set_img_info_cositing(instance, 0);
                intel_vvp_protocol_conv_enable(instance, true);

                TRACE << "[LTF DP Tx Protocol Conv]:\t" << LOG_IO(outputDetails, outputDetails) << '\n' << std::flush;

                _protoConvLTFDpTxCore->_currentInputVideoDetails = outputDetails;
                _protoConvLTFDpTxCore->_currentOutputVideoDetails = outputDetails;

                _protoConvLTFDpTxCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);
            }

            return outputDetails;
        };

        _protoConvLTFDpTxCore = PipelineCore::Create(_spProtoConvLTFDpTx, nullptr, pipelineOutputHandler);

        }
    else
    {
        ERR << "Failed to instantiate DP Tx protocol converter!\n";
    }


    bool initialisedMandatory = inputTpgFound && bayerSwitchFound;

    if (!initialisedMandatory)
    {
        FATAL << "Failed to start above pipeline cores, terminating application.\n";
        exit(1);
    }

    for(const auto& input: _inputs)
    {
        _spInputState->_inputVideoStandard.emplace(input, VideoStandard{});
    }
}

UdxVvpIspPipeline::~UdxVvpIspPipeline()
{
}


bool UdxVvpIspPipeline::SelectInput(const uint32_t idx)
{
    bool ret = false;
    const std::lock_guard<std::recursive_mutex> lock(_mutex);

    if(idx < _inputs.size())
    {
        const PipelineInput& input = _inputs[idx];

        if(_spVfrInput)
        {
            _spVfrInput->Run(input == PipelineInput::FR);
        }

        // Input switch ports
        enum {ECFG_RGBIN = 0, ECFG_BAYERIN, ECFG_DPIN, ECFG_MAX};

        using switch_input_cfg_t = std::array<std::size_t, ECFG_MAX>;
        using video_input_cfg_t = std::pair<PipelineInput, switch_input_cfg_t>;

        // Input ports for the following video switches: [rgb, bayer, dp]
        std::vector<video_input_cfg_t> inputSwitchConfigs {
            {PipelineInput::TPG,       {1,0,1}},
        };

        PipelineInput cameraInput = PipelineInput::Camera0;
        for(const auto& c : _cameras)
        {
            if(c->GetMIPIInterface() == 0)
            {
                inputSwitchConfigs.emplace_back(video_input_cfg_t{cameraInput,   {0,1,1}});
            }
            else
            {
                inputSwitchConfigs.emplace_back(video_input_cfg_t{cameraInput,   {0,2,1}});
            }
            cameraInput = PipelineInput::Camera1;
        }

        inputSwitchConfigs.emplace_back(video_input_cfg_t{PipelineInput::FR,   {0,3,1}});

        TRACE << "Active input: " << InputToString(input) << "\n" << std::flush;
        for(uint32_t pipeline_index = 0; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
        {
            for(const auto& inputSwitchCfg: inputSwitchConfigs)
            {
                if((inputSwitchCfg.first == input) || 
                    ((PipelineInput::Cameras == input) && (inputSwitchCfg.first == (pipeline_index == 0 ? PipelineInput::Camera0 : PipelineInput::Camera1)))
                )
                {
                    const auto& cfg = inputSwitchCfg.second;

                    bool switchOk = true;

                    if(_spRgbSwitch)
                        switchOk = switchOk && _spRgbSwitch->ConnectOnly(cfg[ECFG_RGBIN], 0);

                    if(_spBayerSwitch)
                        switchOk = switchOk && _spBayerSwitch->ConnectOnly(cfg[ECFG_BAYERIN], pipeline_index);

                    if(switchOk)
                    {
                        _spInputState->_input = input;
                        ReprogramPipeAction(UdxVvpIspPipeline::PipelineStateChanged::PreWarp);
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}


bool UdxVvpIspPipeline::SelectOutput(const uint32_t idx)
{
    bool ret = false;

    static constexpr uint32_t OUTPUT_ISP = 0;
    static constexpr uint32_t OUTPUT_TPG = 1;
    static constexpr uint32_t MAX_OUTPUT_SOURCES = 2; /* ISP and Output TPG */

    (void)OUTPUT_ISP; // unused

    if(idx < MAX_OUTPUT_SOURCES)
    {

        static bool lut1dBypass = _spLut1d->GetBypass();

        if(idx == OUTPUT_TPG)
        {
            _outputSrouce = OutputSource::ColorBars;

            intel_vvp_tpg_pattern pattern;
            pattern.type = kIntelVvpTpgBarsPattern;
            pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
            _spOutputMixerTpg->SetSelectedPattern(pattern);

            lut1dBypass = _spLut1d->GetBypass();
            _spLut1d->SetBypass(true);
            _spOutputMixer->HideIsp(true);
            _spOutputMixer->HideOverlay(true);
            _spOutputMixer->HideLogo(true);
        }
        else
        {
            _outputSrouce = OutputSource::ISP;

            _spLut1d->SetBypass(lut1dBypass);
            _spOutputMixer->HideIsp(false);
            _spOutputMixer->HideOverlay(false);
            _spOutputMixer->HideLogo(false);
            
            intel_vvp_tpg_pattern pattern;
            pattern.type = kIntelVvpTpgUniformPattern;
            pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
            _spOutputMixerTpg->SetSelectedPattern(pattern);   
            _spOutputMixerTpg->SetColorsForUniformPattern(0x0, 0x0, 0x0);
        }
    }

    return ret;
}


void UdxVvpIspPipeline::OnScreensaver(bool activate)
{
    if(activate)
    {
        _spOutputMixer->HideIsp(true);
        _spOutputMixer->HideOverlay(true);
        _spOutputMixer->HideLogo(false);

        intel_vvp_tpg_pattern pattern;
        pattern.type = kIntelVvpTpgUniformPattern;
        pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
        _spOutputMixerTpg->SetSelectedPattern(pattern);   
        _spOutputMixerTpg->SetColorsForUniformPattern(0x0, 0x0, 0x0); // Black background for screensaver
    }
    else
    {
        if(_outputSrouce == OutputSource::ISP)
        {
            _spOutputMixer->HideIsp(false);
            _spOutputMixer->HideOverlay(false);
            _spOutputMixer->HideLogo(false);
        }
        else
        {
            _spOutputMixer->HideIsp(true);
            _spOutputMixer->HideOverlay(true);
            _spOutputMixer->HideLogo(true);

            intel_vvp_tpg_pattern pattern;
            pattern.type = kIntelVvpTpgBarsPattern;
            pattern.color = eIntelVvpTpgPatternColor::kIntelVvpTpgRgb;
            _spOutputMixerTpg->SetSelectedPattern(pattern);               
        }
    }
}


PipelineInput UdxVvpIspPipeline::GetUiSelectedInput()
{
    return _spInputState->_input;
}


void UdxVvpIspPipeline::ReadSettingsPIO()
{
    const uint32_t PIO_SETTINGS_ID = PipelineBaseUID(PipelineSubsystem::Misc);
    const uint32_t PIO_BUILDINFO_ID = PipelineBaseUID(PipelineSubsystem::Misc)+1;

    {
        std::string build_time{"FPGA build time: "};

        auto pio = _spHapi->CreateByUniqueID<Hapi::Pio>(PIO_BUILDINFO_ID);

        if(pio)
        {
            uint32_t reg = intel_pio_read(pio->GetInstance());
            const std::time_t buildTime = static_cast<std::time_t>(reg);
            build_time += std::asctime(std::localtime(&buildTime));
        }
        else
        {
            build_time += "N/A\n";
        }

        INFO << build_time;
    }

    {
        auto pio = _spHapi->CreateByUniqueID<Hapi::Pio>(PIO_SETTINGS_ID);

        if(pio)
        {
            constexpr auto PIO_FRAMERATE = [](const uint32_t x) -> uint32_t { return (x & 0x1); };
            constexpr auto PIO_SENSOR_PIPE_ARRANGEMENT = [](const uint32_t x) -> uint32_t { return ((x & 0x2) >> 1); };
            constexpr auto PIO_FW_SUPPORTS_HDR = [](const uint32_t x) -> uint32_t { return ((x & 0x4) >> 2); };
            constexpr auto PIO_BOARD_ID = [](const uint32_t x) -> uint32_t { return ((x & 0xf0) >> 4); };
            constexpr auto PIO_NUM_SENSORS = [](const uint32_t x) -> uint32_t { return ((x & 0xf00) >> 8); };
            constexpr auto PIO_EMIF_CALIBRATION_PENDING = [](const uint32_t x) -> uint32_t { return ((x & 0x80000000) >> 31); };

            (void)PIO_SENSOR_PIPE_ARRANGEMENT;
            (void)PIO_FW_SUPPORTS_HDR;

            uint32_t reg = intel_pio_read(pio->GetInstance());

            uint32_t fpga_emif_calibration_wait_count = 100;    // Wait 10s then give up and warn

            while(PIO_EMIF_CALIBRATION_PENDING(reg))
            {
                if(fpga_emif_calibration_wait_count == 0)
                    break;

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                reg = intel_pio_read(pio->GetInstance());
                --fpga_emif_calibration_wait_count;
            }

            if(PIO_EMIF_CALIBRATION_PENDING(reg))
                WARN << "FPGA EMIF calibration has not been done!\n";

            _boardId = static_cast<BoardType>(PIO_BOARD_ID(reg));

            _framerate = PIO_FRAMERATE(reg) ? (6000) : (3000);
            _numberOfSensors = PIO_NUM_SENSORS(reg) + 1;

            // The following boards use Sony-IMX477 (RPi) camera module which
            // needs to be powered up manually by setting a PIO bit
            if((_boardId == AlteraPremium) || (_boardId == Axe5Eagle) || (_boardId == MacnicaSulfur) || (_boardId == TerasicDE25) || (_boardId == TerasicDE25Nano))
            {
                intel_pio_write(pio->GetInstance(), 0x1);
                // Wait more than 300 ms for the sensor and sensor module circuitry to finish the reset sequence
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            }
        }
        else
        {
            ERR << "Failed to obtain settings PIO!\n";
            return;
        }
    }
}


void UdxVvpIspPipeline::LoadFuncsForUiControls(
    std::function<void(const ImageConfig&)> updateInputConfigCB,
    std::function<void(const ImageConfig&)> updateOutputConfigCB)
{
    _updateInputConfigCB = std::move(updateInputConfigCB);
    _updateOutputConfigCB = std::move(updateOutputConfigCB);
}


void UdxVvpIspPipeline::LoadFuncsForUiFeedback(std::function<void(ImageConfig&)> warpInputSettingCB,
                                               std::function<void(ImageConfig&)> mixerInputSettingCB,
                                               std::function<void(ImageConfig&)> postPipelineProgramUiUpdateCB)
{
    _warpInputSettingCB = std::move(warpInputSettingCB);
    _mixerInputSettingCB = std::move(mixerInputSettingCB);
    _postPipelineProgramUiUpdateCB = std::move(postPipelineProgramUiUpdateCB);
}

bool UdxVvpIspPipeline::Start()
{
    if (!_spHapi || !_spInputState || !_spOutputState)
    {
        return false;
    }

    // Monitor output sink for supported output formats
    if (IsDpTxMultirateEnabled())
    {
        // HDMI Output Initialisation
        // Separate queue thread for reading the HDMI output as it is slow reading
        // from i2c
        _watchOutputQueue = std::make_shared<AtUtils::MessageQueue>("OutputQ");
        if (!_watchOutputQueue)
        {
            return false;
        }

        AtUtils::VariableRepeatCommandQueueCB watchOutputCB =
            [this]() -> int32_t
            {
                WatchOutputTask();
                return _WATCH_OUTPUT_INTERVAL_MS;
            };

        _watchOutputQueue->Add(watchOutputCB, _WATCH_OUTPUT_INTERVAL_MS);
    }
    else // Alterantively use fixed output resolution
    {
			// Either FullHD for TerasicDE25 or 4K for all other boards
            auto vs = ((_boardId == TerasicDE25 || _boardId == TerasicDE25Nano) ?  ImageConfig1080p : ImageConfig2160p);
            vs._frame_rate = _framerate;

            const auto vs_str = vs.ToString();

            auto updateVsCmd = [this, vs = std::move(vs)](){
                SetOutputVideoStandard(vs);
            };

            VvpIspDemo::Get()->AddCommand(updateVsCmd);

            // Also update UI here
            auto updateUiCmd = [vs_str = std::move(vs_str)](){
                VvpIspDemo::Get()->GetUi()->UpdateOutputStatus(0, vs_str, {}, false);
            };

            VvpIspDemo::Get()->AddCommand(updateUiCmd);
    }

    AtUtils::VariableRepeatCommandQueueCB watchInputCB = [this]() -> int32_t {
        WatchInput();
        return 250;
    };

    VvpIspDemo::Get()->AddCommand(watchInputCB, 250);

    for(auto coreIspPipeline: _coreIspPipelines)
        coreIspPipeline->Start();

    INFO << "UDX pipeline started\n";

    return true;
}

void UdxVvpIspPipeline::Stop()
{
}

void UdxVvpIspPipeline::StartShutdown()
{
    // NOTE We must call StartShutdown here, otherwise this'll infinite loop.
    _watchOutputQueue.reset();
}

// State change triggers

void UdxVvpIspPipeline::WarpEnabled(uint32_t pipeline_index, bool state)
{
    auto& coreIspPipeline  = _coreIspPipelines[pipeline_index];

    if(coreIspPipeline && coreIspPipeline->GetWarpAdapter())
    {
        coreIspPipeline->GetWarpAdapter()->SetBypass(!state);
    }
    //ReprogramPipeAction(PipelineStateChanged::PostWarp);
}


bool UdxVvpIspPipeline::IsActiveInputValid(uint32_t pipeline_index) const
{
    auto& coreIspPipeline = _coreIspPipelines[pipeline_index];
    VideoStandard currentInputVideoInfo = coreIspPipeline->GetInputCore()->GetCurrentInputVideoInfo();
    return currentInputVideoInfo.IsValid();
}


void UdxVvpIspPipeline::EnableIspMixerChannel(uint32_t pipeline_index, bool enable)
{
    if(_spOutputMixer)
        _spOutputMixer->EnableIsp(enable);
}


void UdxVvpIspPipeline::ReprogramPipePreWarp(bool resetClipper)
{
    if (_spRgbSwitch)
    {
        auto spRgbSwitchUi = std::dynamic_pointer_cast<Switch::SwitchUiStateImplementation>(_spRgbSwitch);
        spRgbSwitchUi->ApplyUiState();
    }
    if (_spBayerSwitch)
    {
        auto spBayerSwitchUi = std::dynamic_pointer_cast<Switch::SwitchUiStateImplementation>(_spBayerSwitch);
        spBayerSwitchUi->ApplyUiState();
    }

    // Propagate new video standard only on the active video input
    // Ignoring other
    const auto& selectedInput = GetUiSelectedInput();
    VideoStandard newInputVideoInfo = _spInputState->_inputVideoStandard[selectedInput];

    for(uint32_t pipeline_index = 0; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
    {
        auto& coreIspPipeline = _coreIspPipelines[pipeline_index];

        uint32_t mipi_idx = 0xFFFFFFFF;
        switch (selectedInput)
        {
            case PipelineInput::Cameras:
                mipi_idx = pipeline_index;
            break;
            case PipelineInput::Camera0:
                mipi_idx = 0;
            break;
            case PipelineInput::Camera1:
                mipi_idx = 1;
            break;
            default:
            break;
        }

        switch (selectedInput)
        {
            case PipelineInput::Camera0:
            case PipelineInput::Camera1:
            case PipelineInput::Cameras:
            {
                if(mipi_idx < _cameras.size())
                {
                    const auto pCamera = _cameras[mipi_idx];
                    coreIspPipeline->SetInputCfaPhase(pCamera->GetCfaPhase());
                    if(newInputVideoInfo.IsValid())
                    {
                        auto cameraPipelineCore = _sensorIntfCore[mipi_idx];
                        cameraPipelineCore->ApplyInputVideoInfo(newInputVideoInfo);
                    }
                }
            }
            break;
            case PipelineInput::TPG:
            {
                TCfaPhase inputCfaPhase = TCfaPhase::BGGR;
                if(_coreIspPipelines.size() > 0)
                {
                    inputCfaPhase = _spRemosaic ? coreIspPipeline->GetDemosaic()->GetCfaPhase() : TCfaPhase::BGGR;
                }
                coreIspPipeline->SetInputCfaPhase(inputCfaPhase);

                // Remosaic CFA phase tracks demosaic
                if(_spRemosaic)
                {
                    // This is a temporary fix. Pending investigation
                    if(_boardId == AlteraModular)
                        _spRemosaic->SetCfaPhase(TCfaPhase::RGGB);
                    else
                        _spRemosaic->SetCfaPhase(inputCfaPhase);
                }

                std::shared_ptr<Tpg::TpgUiStateImplementation> spTpgUi = std::dynamic_pointer_cast<Tpg::TpgUiStateImplementation>(_spTpg);

                spTpgUi->ApplyUiStateResolution();
                if(newInputVideoInfo.IsValid())
                {
                    _tpgCore->ApplyInputVideoInfo(newInputVideoInfo);
                }
            }
            break;
            case PipelineInput::FR:
            {
                _spInputState->_inputCfaPhase = TCfaPhase::RGGB;
                _vfrInputCore->ApplyInputVideoInfo(newInputVideoInfo);
            }
            break;        
            default:
                break;
        }

        VideoStandard currentInputVideoInfo = coreIspPipeline->GetInputCore()->GetCurrentInputVideoInfo();

        // If ISP video input changed hide output until pipeline reconfigured
        if(newInputVideoInfo != currentInputVideoInfo)
        {
            TRACE << "Pipeline " << pipeline_index << " input changed to: " << newInputVideoInfo << "\n" << std::flush;

            EnableIspMixerChannel(pipeline_index, false);
            _spInputState->_inputVideoStandardChanged = true;

            if(newInputVideoInfo.IsValid())
            {
                if(resetClipper)
                {
                    coreIspPipeline->ResetClipper(newInputVideoInfo._width, newInputVideoInfo._height);
                }
                else
                {
                    coreIspPipeline->UpdateClipper(newInputVideoInfo._width, newInputVideoInfo._height);
                }
            }
        }
    }
}


void UdxVvpIspPipeline::ReprogramPipePostWarp()
{
    for(uint32_t pipeline_index = 0; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
    {
        auto& coreIspPipeline = _coreIspPipelines[pipeline_index];
        ImageConfig outputConfig{};

        outputConfig._width = _spOutputState->_outputVideoStandard.Width();
        outputConfig._height = _spOutputState->_outputVideoStandard.Height();
        outputConfig._frame_rate = _spOutputState->_outputVideoStandard.FrameRate();             

        if (coreIspPipeline->GetOutputCore())
        {
            coreIspPipeline->GetOutputCore()->ApplyInputVideoInfo(outputConfig);
        }

        if (_mixerInputSettingCB)
        {
            _mixerInputSettingCB(outputConfig);
        }
    }
}

void UdxVvpIspPipeline::ReprogramPipePostMixer()
{
    const auto& outputVideoStandard = _spOutputState->_outputVideoStandard;

    if(_terminalCore)
        _terminalCore->ApplyOutputVideoInfo(outputVideoStandard);

    // Now update the UI with the changes
    _updateOutputConfigCB(outputVideoStandard);
}


void UdxVvpIspPipeline::ReprogramPipeAction(const PipelineStateChanged& stateChanged)
{
    static bool hasBeenInitialised = false;

    const std::lock_guard<std::recursive_mutex> lock(_mutex);

    bool reprogramFullPipe = !hasBeenInitialised || stateChanged == PipelineStateChanged::ReprogramWholePipeline;

#ifdef ALWAYS_REPROGRAM_PIPE
    reprogramFullPipe = true;
#endif

    // If we were doing a workaround, that will involve setting specific values for
    // resolutions and whatnot. Because of the optimisations to prevent redundant
    // updates, this can result in some of those settings being "stuck" when moving
    // to a state that shouldn't involve the workarounds as we skip reading the
    // ui state.
    // Therefore, in the case where we leave a workaround state, reprogram the full
    // pipeline to return to a clean slate.
    if (_spOutputState->_isOnWorkaroundState)
    {
        _spOutputState->_isOnWorkaroundState = false;
        reprogramFullPipe = true;
    }


    // We program the pre-mixer stage when:
    // - The input has changed
    bool shouldProgramInput = (stateChanged == PipelineStateChanged::PreWarpNoClipperReset) ||
                              (stateChanged == PipelineStateChanged::PreWarp) || reprogramFullPipe;

    // We program the warp->mixer stage when:
    // - Warp's output has changed, OR
    // - Warp is set to follow_output, and the HDMI has changed
    bool shouldProgramWarp =  (((stateChanged == PipelineStateChanged::PostWarp) ||
                               (stateChanged == PipelineStateChanged::PostMixer)) || reprogramFullPipe) && (_coreIspPipelines.size() == 1);

    bool shouldProgramMixer = ((stateChanged == PipelineStateChanged::PostMixer) || shouldProgramWarp) || reprogramFullPipe;
    if(!_spOutputState->_outputVideoStandard.IsValid())
    {
        shouldProgramMixer = false;
    }

    if (shouldProgramInput)
    {
        bool shouldResetClipper = (reprogramFullPipe) || (stateChanged != PipelineStateChanged::PreWarpNoClipperReset);
        ReprogramPipePreWarp(shouldResetClipper);
    }

    if (shouldProgramMixer)
    {
        ReprogramPipePostMixer();
    }

    if (shouldProgramWarp)
    {
        ReprogramPipePostWarp();
    }

    if (!hasBeenInitialised)
    {
        hasBeenInitialised = true;
    }

    if (_postPipelineProgramUiUpdateCB)
    {
        _postPipelineProgramUiUpdateCB(_spOutputState->_mixerOutput);
    }

    if(_spInputState->_inputVideoStandardChanged)
    {
        for(uint32_t pipeline_index = 0; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
        {
            if(IsActiveInputValid(pipeline_index))
            {
                EnableIspMixerChannel(pipeline_index, true);
            }

            _spInputState->_inputVideoStandardChanged = false;
        }
    }
}


bool UdxVvpIspPipeline::InitCameras()
{
    static constexpr uint32_t NUM_MIPI_MAX = 4;
    const uint32_t numMipiIfs = std::min(_numberOfSensors, NUM_MIPI_MAX);
    uint8_t mipiIfs[NUM_MIPI_MAX] = {0};

#ifdef CAMERA_FramosGMSL
    bool gmslFound = false;

    for (uint32_t idx = 0; idx < numMipiIfs; ++idx)
    {
        // Detect GMSL serializer/deserializers present
        if(SwApi::FramosGMSL::Create(idx))
        {
            TRACE << "Found GMSL device on MIPI" << idx << '\n' << std::flush;
            gmslFound = true;
        }
    }

    if(!gmslFound)
        TRACE << "No GMSL devices detected\n" << std::flush;
#endif /* CAMERA_FramosGMSL */

    // Discover cameras
    for (uint32_t idx = 0; idx < numMipiIfs; ++idx)
    {
        {
            // Exposure fusion cores
            const uint32_t EXP_FUSION_UID = PipelineBaseUID(PipelineSubsystem::Camera, idx);
            Hapi::VvpExposureFusionPtr spExpFusion = _spHapi->CreateByUniqueID<Hapi::VvpExposureFusion>(EXP_FUSION_UID);
            if (spExpFusion)
            {
                auto instance = spExpFusion->GetInstance();
                intel_vvp_exposure_fusion_set_run_mode(instance, kIntelVvpExposureFusionModeLongExposure);
                intel_vvp_exposure_fusion_set_black_level(instance, 200);
                intel_vvp_exposure_fusion_set_exposure_ratio(instance, 39086);
                intel_vvp_exposure_fusion_set_threshold(instance, 0x1050);

                _spInputExposureFusion.emplace_back(spExpFusion);

                auto exposureFusionPipelineHandler = [this, idx](const ImageConfig& inputDetails) -> ImageConfig {
                    const std::lock_guard<std::recursive_mutex> lock(_mutex);
                    const auto exposureFusionCore = _exposureFusionCore[idx];

                    ASSERT(exposureFusionCore->_outputCores.size() == 1, "Exposure fuision only supports a single output.");

                    exposureFusionCore->_currentInputVideoDetails = inputDetails;
                    exposureFusionCore->_currentOutputVideoDetails = inputDetails;
                    // Exposure Fusion output is 16 bit
                    exposureFusionCore->_currentOutputVideoDetails.SetColourDepth(TColourDepth::BPC16);

                    const auto& outputDetails = exposureFusionCore->_currentOutputVideoDetails;

                    TRACE << "[Exposure Fusion " << idx <<"]:\t" << LOG_IO(inputDetails, outputDetails) << '\n' << std::flush;

                    exposureFusionCore->_outputCores[0]->ApplyInputVideoInfo(outputDetails);

                    return outputDetails;
                };

                const auto exposureFusionCore = PipelineCore::Create(spExpFusion, exposureFusionPipelineHandler);
                _exposureFusionCore.emplace_back(exposureFusionCore);
            }
            else
            {
                _spInputExposureFusion.emplace_back(nullptr);
                _exposureFusionCore.emplace_back(nullptr);
            }
        }

        const uint32_t CAMERA_SNOOP_UID = PipelineBaseUID(PipelineSubsystem::Camera, idx);
        auto inputSnoop = _spHapi->CreateByUniqueID<Hapi::VvpSnoop>(CAMERA_SNOOP_UID);

        if(inputSnoop)
            TRACE << "Found VVP Snoop " <<  idx << "\n" << std::flush;

        _spInputSnoop.emplace_back(inputSnoop);

        // If MIPI interface is not taken
        if( mipiIfs[idx] == 0)
        {
            ICameraPtr cam;
            for(const auto& camera: cameraList)
            {
                if(camera.create != nullptr)
                {
                    cam = camera.create(idx, _framerate);
                    if(cam)
                    {
                        break;
                    }
                }
            }
            if (cam)
            {
                _cameras.emplace_back(cam);
                // Mark MIPI interface as taken
                mipiIfs[idx] = 1;
                continue;
            }
        }
    }

    if(_cameras.size() < numMipiIfs)
    {
        // Discover other cameras here same way as above
    }

    // Create Pipeline Core instances for each camera
    for(const auto& cam: _cameras)
    {
        // Create Pipeline Core instance for the camera
        const auto idx = _sensorIntfCore.size();

        TRACE << "Found camera: " << _cameras[idx]->GetModel() << ". Enumerated as [Cam " << idx << "]\n" << std::flush;
        TRACE << "Exposure fusion: " << (_spInputExposureFusion[idx] ? "found" : "not available") << "\n" << std::flush;

        auto sensorIntfPipelineHandler = [this, idx](const ImageConfig& inputDetails) -> ImageConfig {
                const std::lock_guard<std::recursive_mutex> lock(_mutex);
                const auto sensorIntfCore = _sensorIntfCore[idx];

                ASSERT(sensorIntfCore->_outputCores.size() == 1, "The Camera Intf only supports a single output.");


                TRACE << "[Camera " << idx << "]:\t" << "Out: " << inputDetails << "\n" << std::flush;

                sensorIntfCore->_currentInputVideoDetails = inputDetails;
                sensorIntfCore->_currentOutputVideoDetails = inputDetails;
                sensorIntfCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

                return inputDetails;
        };

        const auto sensorIntfCore = PipelineCore::Create(cam, sensorIntfPipelineHandler);
        _sensorIntfCore.emplace_back(sensorIntfCore);
    }

    // Initialise MIPI PiP converters separately
    for(uint32_t idx = 0; idx < numMipiIfs; ++idx)
    {
        if(mipiIfs[idx])
        {
            bool pipConvOk = InitInputPipConv(idx);

            if(!pipConvOk)
                ERR << "Failed to initialise MIPI PiP converter " << idx << "\n" ;
        }
    }

    return !_cameras.empty();
}


bool UdxVvpIspPipeline::InitInputPipConv(const uint32_t id)
{
    const uint32_t OUTPUT_PIP_UID = PipelineBaseUID(PipelineSubsystem::Camera, id);
    auto spInputPipConv = _spHapi->CreateByUniqueID<Hapi::VvpPipConv>(OUTPUT_PIP_UID);

    if(!spInputPipConv)
        return false;

    auto spInstance = spInputPipConv->GetInstance();

    int err = intel_vvp_pip_conv_init(spInstance, spInstance->core_instance.base);

    if (err != kIntelVvpCoreOk )
    {
        TRACE << "Failed to intialise Input Pip Converter instance\n" << std::flush;
        return false;
    }

    intel_vvp_core_set_img_info_width(spInputPipConv->GetInstance(), 3840);
    intel_vvp_core_set_img_info_subsampling(spInputPipConv->GetInstance(), 3);

    const std::size_t idx = _spInputPipConv.size();

    // Pipeline core
    auto inputPipConvCorePipelineHandler =
        [this, idx](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);

            ASSERT(_inputPipConvCore[idx]->_inputCores.size() == 1 && _inputPipConvCore[idx]->_outputCores.size() == 1);

            TRACE << "[Input PiP Conv]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

            intel_vvp_core_set_img_info_width(_spInputPipConv[idx]->GetInstance(), inputDetails._width);

            _inputPipConvCore[idx]->_currentInputVideoDetails = inputDetails;
            _inputPipConvCore[idx]->_currentOutputVideoDetails = inputDetails;
            _inputPipConvCore[idx]->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };

    _spInputPipConv.push_back(spInputPipConv);
    _inputPipConvCore.push_back(PipelineCore::Create(spInputPipConv, inputPipConvCorePipelineHandler));

    return true;
}


bool UdxVvpIspPipeline::InitOutputPipConv()
{
    const uint32_t OUTPUT_PIP_UID = PipelineBaseUID(PipelineSubsystem::Output);
    _spOutputPipConv = _spHapi->CreateByUniqueID<Hapi::VvpPipConv>(OUTPUT_PIP_UID);
    if (!_spOutputPipConv) return false;

    auto spInstance = _spOutputPipConv->GetInstance();

    int err = intel_vvp_pip_conv_init(spInstance, spInstance->core_instance.base);
    if (err != kIntelVvpCoreOk )
    {
        TRACE << "Failed to intialise Input Pip Converter instance\n" << std::flush;
        return false;
    }

    intel_vvp_core_set_img_info_width(_spOutputPipConv->GetInstance(), 3840);
    intel_vvp_core_set_img_info_subsampling(_spOutputPipConv->GetInstance(), 3);

    // Pipeline core
    auto outputPipConvCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);

            ASSERT(_outputPipConvCore->_inputCores.size() == 1);

            if(!_outputPipConvCore->_outputCores.empty())
            {
                ASSERT(_outputPipConvCore->_outputCores.size() == 1);
            }

            TRACE << "[Output Pip Conv]:\t" << LOG_IO(outputDetails, outputDetails) << '\n' << std::flush;

            intel_vvp_core_set_img_info_width(_spOutputPipConv->GetInstance(), outputDetails._width);

            _outputPipConvCore->_currentInputVideoDetails = outputDetails;
            _outputPipConvCore->_currentOutputVideoDetails = outputDetails;

            _outputPipConvCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);

            return outputDetails;
        };

    _outputPipConvCore = PipelineCore::Create(_spOutputPipConv, nullptr, outputPipConvCorePipelineOutputHandler);

    return true;
}


bool UdxVvpIspPipeline::InitVfwCaptureRaw()
{
    const uint32_t VFW_CAPURE_RAW_UID = PipelineBaseUID(PipelineSubsystem::CoreISP) + 0;

    auto spVvpVfwCaptureRaw = _spHapi->CreateByUniqueID<Hapi::VvpVfw>(VFW_CAPURE_RAW_UID);

    if(!spVvpVfwCaptureRaw)
    {
        WARN << "Raw capture Video Frame Writer not available\n";
        return false;
    }

    ::intel_vvp_core_set_img_info_interlace(spVvpVfwCaptureRaw->GetInstance(), 0);
    ::intel_vvp_core_set_img_info_subsampling(spVvpVfwCaptureRaw->GetInstance(), 0x3);
    ::intel_vvp_core_set_img_info_cositing(spVvpVfwCaptureRaw->GetInstance(), 0);
    ::intel_vvp_core_set_img_info_colorspace(spVvpVfwCaptureRaw->GetInstance(), 0);

    // Default frame capturer resolution
    VideoStandard vfwImageParams = ImageConfig2160p;

    if((_boardId == TerasicDE25) || (_boardId == TerasicDE25Nano))
    {
        // DDR shared between HPS and FPGA
        // Capture buffer starts at predefined offset of 32Mb
        // after the VVP Framebuffer space
        static const uintptr_t BUFFER_ADDR_FPGA = 0x02000000;
        static const uintptr_t BUFFER_ADDR_CPU  = 0x82000000;

        _spVfwCaptureRaw = std::make_shared<VideoFrameWriter>(spVvpVfwCaptureRaw, BUFFER_ADDR_CPU, BUFFER_ADDR_FPGA);

        vfwImageParams = ImageConfig1080p;
    }
    else
    {
        // Use FPGA DDR on EMIF2
        static const uintptr_t BUFFER_ADDR_FPGA = 0x0000000000;
        static const uintptr_t BUFFER_ADDR_DMA  = 0x0200000000;          

         _spVfwCaptureRaw = std::make_shared<VideoFrameWriter>(spVvpVfwCaptureRaw, BUFFER_ADDR_DMA, BUFFER_ADDR_FPGA);
    }

    if (!_spVfwCaptureRaw)
        return false;

    _spVfwCaptureRaw->SetResolutionOfImage(vfwImageParams.Width(), vfwImageParams.Height());        

    const uint32_t CPM_UID = PipelineBaseUID(PipelineSubsystem::CoreISP);
    _spVvpCpm = _spHapi->CreateByUniqueID<Hapi::VvpCpm>(CPM_UID);
    if (!_spVvpCpm) return false;

    ::intel_vvp_core_set_img_info_interlace(_spVvpCpm->GetInstance(), 0);
    ::intel_vvp_core_set_img_info_subsampling(_spVvpCpm->GetInstance(), 0x3);
    ::intel_vvp_core_set_img_info_cositing(_spVvpCpm->GetInstance(), 0);
    ::intel_vvp_core_set_img_info_colorspace(_spVvpCpm->GetInstance(), 0);

    return true;
}


bool UdxVvpIspPipeline::InitVfwCaptureIsp()
{
    const uint32_t VFW_CAPURE_ISP_UID = PipelineBaseUID(PipelineSubsystem::Output) + 0;

    auto spVvpVfwCaptureIsp = _spHapi->CreateByUniqueID<Hapi::VvpVfw>(VFW_CAPURE_ISP_UID);

    if(!spVvpVfwCaptureIsp)
    {
        WARN << "ISP capture Video Frame Writer not available\n";
        return false;
    }

    ::intel_vvp_core_set_img_info_interlace(spVvpVfwCaptureIsp->GetInstance(), 0);
    ::intel_vvp_core_set_img_info_subsampling(spVvpVfwCaptureIsp->GetInstance(), 0x3);
    ::intel_vvp_core_set_img_info_cositing(spVvpVfwCaptureIsp->GetInstance(), 0);
    ::intel_vvp_core_set_img_info_colorspace(spVvpVfwCaptureIsp->GetInstance(), 0); 

    // Default frame capturer resolution
    VideoStandard vfwImageParams = ImageConfig2160p;

    if((_boardId == TerasicDE25) || (_boardId == TerasicDE25Nano))
    {
        // DDR shared between HPS and FPGA
        // Capture buffer starts at predefined offset of 32Mb
        // after the VVP Framebuffer space
        static const uintptr_t BUFFER_ADDR_FPGA = 0x02000000;
        static const uintptr_t BUFFER_ADDR_CPU  = 0x82000000;

        _spVfwCaptureIsp = std::make_shared<VideoFrameWriter>(spVvpVfwCaptureIsp, BUFFER_ADDR_CPU, BUFFER_ADDR_FPGA);

        vfwImageParams = ImageConfig1080p;
    }
    else
    {
        // Use FPGA DDR on EMIF2
        static const uintptr_t BUFFER_ADDR_FPGA = 0x0000000000;
        static const uintptr_t BUFFER_ADDR_DMA  = 0x0200000000;        

         _spVfwCaptureIsp = std::make_shared<VideoFrameWriter>(spVvpVfwCaptureIsp, BUFFER_ADDR_DMA, BUFFER_ADDR_FPGA);
    }

    if (!_spVfwCaptureIsp)
        return false;

    _spVfwCaptureIsp->SetResolutionOfImage(vfwImageParams.Width(), vfwImageParams.Height());        

    return true;
}


bool UdxVvpIspPipeline::InitTpg()
{
    const uint32_t TPG_UID = PipelineBaseUID(PipelineSubsystem::Input);

    auto spVvpTpgInst = _spHapi->CreateByUniqueID<Hapi::VvpTpg>(TPG_UID);

    if (!spVvpTpgInst)
        return false;

    const uint32_t TPG_WIDTH = ((_boardId == TerasicDE25) || (_boardId == TerasicDE25Nano)) ? 1920 : 3840;
    const uint32_t TPG_HEIGHT = ((_boardId == TerasicDE25) || (_boardId == TerasicDE25Nano)) ? 1080 : 2160;

    _spTpg = std::static_pointer_cast<ITpg>(std::make_shared<Tpg::TpgUiStateImplementation>(spVvpTpgInst, TPG_WIDTH, TPG_HEIGHT));

    if (!_spTpg)
        return false;

    _spTpg->SetSelectedPattern({kIntelVvpTpgBarsPattern, kIntelVvpTpgRgb});
    _spTpg->SetEnable(true);

    // Now the pipeline core
    auto tpgCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_tpgCore->_outputCores.size() == 1);
            ASSERT(_tpgCore->_inputCores.size() == 0, "The VVP TPG does not take input - it's an output-only core.");

            TRACE << "[TPG]:\t" << LOG_V(inputDetails) << "\n" << std::flush;

            _tpgCore->_currentInputVideoDetails = inputDetails;
            _tpgCore->_currentOutputVideoDetails = inputDetails;
            _tpgCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };

    _tpgCore = PipelineCore::Create(_spTpg, tpgCorePipelineHandler);

    const uint32_t TPG_THROTTLE_UID = PipelineBaseUID(PipelineSubsystem::Input);
    auto spTpgThrottleInst = _spHapi->CreateByUniqueID<Hapi::VvpThrottle>(TPG_THROTTLE_UID);
    if(spTpgThrottleInst)
    {
        auto instance = spTpgThrottleInst->GetInstance();
        altera_vvp_throttle_stop(instance);
        altera_vvp_throttle_start(instance);
        const uint32_t frameRate = 6000;
        const uint32_t cpl = (4400 * 3000) / frameRate;
        altera_vvp_throttle_set_clock_cycles_per_line(instance, cpl);
        altera_vvp_throttle_set_active_lines(instance, 2160);
        altera_vvp_throttle_set_v_blank_lines(instance, 90);
        altera_vvp_throttle_commit_writes(instance);
    }

    return true;
}


bool UdxVvpIspPipeline::InitVfrInput()
{
    bool ret = false;

    static constexpr uint32_t FR_UID = 0x14;

    auto spVfrHapi = _spHapi->CreateByUniqueID<Hapi::VvpVfr>(FR_UID);

    if(spVfrHapi)
    {
        INFO << "Frame Reader HAPI instance created with unique ID " << FR_UID << "\n";

        const uint32_t VFR_INPUT_THROTTLE_UID = PipelineBaseUID(PipelineSubsystem::Input) + 1;

        auto vfrInputThrottleHapi = _spHapi->CreateByUniqueID<Hapi::VvpThrottle>(VFR_INPUT_THROTTLE_UID);

        std::unique_ptr<VideoThrottle> spVfrInputThrottle{nullptr};
        
        if(vfrInputThrottleHapi)
        {
            try
            {
                static constexpr uint32_t VIDEO_THROTLE_PIXEL_CLOCK = 297000000; // 594MHz, 2 pixels per clock
                spVfrInputThrottle = std::make_unique<VideoThrottle>(vfrInputThrottleHapi, VIDEO_THROTLE_PIXEL_CLOCK);            
            }
            catch(const std::exception& e)
            {
                std::cerr << "Unable to create Frame Reader Video Throttle: " << e.what() << '\n';
            }
        }        

        // HW design uses 2GB of the second DDR bank
        // 0x0010000000 (256MB) is reserved for the capture framewriters
        // 0x0070000000 (1792MB) is the remaining space allocated for the VFR input
        // 0x0200000000 is the offset at which MSGDMA maps the second DDR bank
        
        static constexpr uintptr_t BUFFER_OFFSET_DMA    = 0x0210000000;
        static constexpr uintptr_t BUFFER_OFFSET_FPGA   = 0x0010000000;
        static constexpr uint32_t BUFFER_SIZE_MAX       = 0x0070000000;

        try
        {
            _spVfrInput = std::make_shared<SwApi::IspVfrInput>(spVfrHapi, BUFFER_OFFSET_DMA, BUFFER_OFFSET_FPGA, BUFFER_SIZE_MAX, std::move(spVfrInputThrottle));
        }
        catch(const std::exception& e)
        {
            ERR << "Failed to create VFR input instance: " << e.what() << "\n";
        }

        if(_spVfrInput)
        {
            auto updateInputFrameReaderControlsCb = [this](const VfrSourceMetadata& metadata){
                VvpIspDemo::Get()->GetUi()->UpdateInputFrameReaderControls(metadata);
            };

            _spVfrInput->SetSourceMetadataCallback(updateInputFrameReaderControlsCb);

            _inputs.emplace_back(PipelineInput::FR);

            // Now the pipeline core
            auto frCorePipelineHandler = [this](const ImageConfig& inputDetails) -> ImageConfig {
                const std::lock_guard<std::recursive_mutex> lock(_mutex);
                
                ASSERT(_vfrInputCore->_outputCores.size() == 1);
                ASSERT(_vfrInputCore->_inputCores.size() == 0, "The VVP Frame Reader does not take input - it's an output-only core.");

                TRACE << "[FR]:\t" << LOG_V(inputDetails) << "\n" << std::flush;

                _vfrInputCore->_currentInputVideoDetails = inputDetails;
                _vfrInputCore->_currentOutputVideoDetails = inputDetails;
                _vfrInputCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

                return inputDetails;
            };

            _vfrInputCore = PipelineCore::Create(_spVfrInput, frCorePipelineHandler);

            ret = true;
        }
    }
    else
    {
        ERR << "Failed to create Frame Reader HAPI instance with unique ID " << FR_UID << "\n";        
    }

    return ret;
}


bool UdxVvpIspPipeline::InitBayerSwitch()
{
    const uint32_t BAYER_SW_UID = PipelineBaseUID(PipelineSubsystem::Input);

    auto spBayerSwitchInst = _spHapi->CreateByUniqueID<Hapi::VvpSwitch>(BAYER_SW_UID);
    if (!spBayerSwitchInst)
    {
        ERR << "Failed to create Bayer HAPI VvpSwitch with unique ID "
            << BAYER_SW_UID << "\n";
        return false;
    }

    _spBayerSwitch = std::static_pointer_cast<ISwitch>(std::make_shared<Switch::SwitchUiStateImplementation>(spBayerSwitchInst));

    if (!_spBayerSwitch)
    {
        ERR << "Failed to create BAYER ISwitch with unique ID "
            << BAYER_SW_UID << "\n";
        return false;
    }
    auto numCoreIspPipelines = _spBayerSwitch->GetNumberOfOutputsAvailable();

    _coreIspPipelines.resize(numCoreIspPipelines);
    
    for(uint32_t pipeline_index = 0U; pipeline_index <_coreIspPipelines.size(); pipeline_index++)
    {
        _coreIspPipelines[pipeline_index] = std::make_shared<CoreIspPipeline>(pipeline_index, _spHapi, PipelineBaseUID(PipelineSubsystem::CoreISP, pipeline_index));
    }

    // Initialise to route TPG output untill the entire pipeline has been initialised
    // and persisted settings loaded
    static constexpr uint8_t TPG_INPUT = 0;
    _spBayerSwitch->ConnectOnly(TPG_INPUT, 0);

    // Pipeline core
    auto bayerSwitchPipelineHandler =
        [this](const ImageConfig& unknownInputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            ImageConfig switchInputDetails;
            
            for(uint32_t pipeline_index = 0U; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
            {
                auto& coreIspPipeline = _coreIspPipelines[pipeline_index];

                // Keep track of current CFA phase and propagate if changed
                static TCfaPhase cfaPhase = TCfaPhase::RGGB;

                // Look at the currently active input.
                auto inputId = _spBayerSwitch->GetConnectedInputForOutput(pipeline_index);
                // If only MIPI 1 interface connected it will be Camera0
                if(inputId >= _bayerSwitchCore->_inputCores.size())
                {
                    inputId = _bayerSwitchCore->_inputCores.size() - 1;
                }

                PipelineCore* activeInputCore = _bayerSwitchCore->_inputCores[inputId];

                // Grab the active input's output details.
                switchInputDetails = activeInputCore->GetCurrentOutputVideoInfo();

                if ((switchInputDetails._width == 0) || (switchInputDetails._height == 0))
                {
                    WARN << "[Bayer Switch]: Invalid video on input = " << (unsigned int)inputId << '\n';
                    return switchInputDetails;
                }

                if((switchInputDetails != coreIspPipeline->GetInputCore()->_currentInputVideoDetails) or
                    (cfaPhase != coreIspPipeline->GetInputCfaPhase()))
                {
                    TRACE << "[Bayer Switch]:\t" << "Input " << (uint32_t)inputId << "\t" << LOG_IO(unknownInputDetails, switchInputDetails) << '\n' << std::flush;
                    PipelineCore* activeOutputCore = _bayerSwitchCore->_outputCores[pipeline_index];
                    if(activeOutputCore)
                    {
                        // Pass this information to the output core after the switch.
                        activeOutputCore->ApplyInputVideoInfo(switchInputDetails);
                    }
                }
                if((switchInputDetails != coreIspPipeline->GetInputCore()->_currentInputVideoDetails) or
                    (cfaPhase != coreIspPipeline->GetInputCfaPhase()))
                {
                    //coreIspPipeline->GetInputCore()->ApplyInputVideoInfo(switchInputDetails);
                    coreIspPipeline->SetInputCfaPhase(cfaPhase);
                }
            }

            return switchInputDetails;
        };

    _bayerSwitchCore = PipelineCore::Create(_spBayerSwitch, bayerSwitchPipelineHandler);

    return true;
}

bool UdxVvpIspPipeline::InitLut1d()
{
    const uint32_t LUT1D_UID = PipelineBaseUID(PipelineSubsystem::Output);
    auto spVvpLut1d = _spHapi->CreateByUniqueID<Hapi::Vvp1dLut>(LUT1D_UID);
    if (!spVvpLut1d) return false;
    auto spVvpLut1dInst = spVvpLut1d->GetInstance();

    if (intel_vvp_1d_lut_init(spVvpLut1dInst, spVvpLut1dInst->core_instance.base) != kIntelVvpCoreOk ){
        TRACE
            << "Failed to intialise 1D LUT instance\n" << std::flush;
        return false;
    }

    _spLut1d = SwApi::Lut1D::Create(spVvpLut1d);
    if (!_spLut1d) return false;

    if(!_spLut1d->SetResolution(1920u, 1080u))
    {
        TRACE << "Failed to set 1D LUT output resolution\n" << std::flush;
        return false;
    }

    // Pipeline core

    auto lut1dCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_lut1dCore->_inputCores.size() == 1);

            TRACE << "[1D LUT]:\t" << LOG_IO(outputDetails, outputDetails) << '\n' << std::flush;

            _spLut1d->SetResolution(outputDetails._width, outputDetails._height);

            _lut1dCore->_currentInputVideoDetails = outputDetails;
            _lut1dCore->_currentOutputVideoDetails = outputDetails;

            _lut1dCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);

            return outputDetails;
        };
    _lut1dCore = PipelineCore::Create(_spLut1d, nullptr, lut1dCorePipelineOutputHandler);

    return true;
}


bool UdxVvpIspPipeline::InitCrs()
{
    const uint32_t CRS_UID = PipelineBaseUID(PipelineSubsystem::Camera);
    auto spVvpCrsInst = _spHapi->CreateByUniqueID<Hapi::VvpCrs>(CRS_UID);
    if (!spVvpCrsInst)
    {
        return false;
    }

    _spCrs = ICrs::Create(spVvpCrsInst, 1920, 1080);

    if (!_spCrs)
    {
        return false;
    }

    _spCrs->SetInputSubsampling(kIntelVvpCrsSubsampling444);
    _spCrs->SetOutputSubsampling(kIntelVvpCrsSubsampling444);

    // Pipeline core

    auto crsCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_crsCore->_outputCores.size() == 1 && _crsCore->_inputCores.size() == 1,
                    "The VVP CRS must take both input and output.");
            TRACE << "[CRS]:\n"
                  << LOG_V(inputDetails) << "\n" << std::flush;

            // set the input subsampling
            ImageConfig crsSettingDetails = inputDetails;
            _spCrs->SetOutputWidth(inputDetails._width);
            _spCrs->SetOutputHeight(inputDetails._height);

            // Set input and output info blocks
            _crsCore->_currentInputVideoDetails = inputDetails;
            _crsCore->_currentOutputVideoDetails = crsSettingDetails;
            _crsCore->_outputCores[0]->ApplyInputVideoInfo(crsSettingDetails);

            return crsSettingDetails;
        };
    _crsCore = PipelineCore::Create(_spCrs, crsCorePipelineHandler);

    return true;
}

bool UdxVvpIspPipeline::InitCsc()
{
    const uint32_t CSC_UID = PipelineBaseUID(PipelineSubsystem::Camera);
    auto spVvpCscInst = _spHapi->CreateByUniqueID<Hapi::VvpCsc>(CSC_UID);
    if (!spVvpCscInst)
    {
        TRACE
            << "Failed to create CSC\n" << std::flush;
        return false;
    }

    _spCsc = SwApi::ICsc::Create(spVvpCscInst, 1920, 1080);

    if (!_spCsc)
    {
        TRACE
            << "Failed to init ICSC\n" << std::flush;
        return false;
    }

    // Pipeline Core

    auto cscCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_cscCore->_outputCores.size() == 1 && _cscCore->_inputCores.size() == 1,
                    "The VVP CSC must take both input and output.");

            ImageConfig cscSettingDetails = inputDetails;
            _spCsc->SetOutputWidth(inputDetails._width);
            _spCsc->SetOutputHeight(inputDetails._height);

            TRACE << "[CSC]: \n"
                  << "Input: " << LOG_V(inputDetails) << "\n" << std::flush;

            _cscCore->_currentInputVideoDetails = inputDetails;
            _cscCore->_currentOutputVideoDetails = cscSettingDetails;
            _cscCore->_outputCores[0]->ApplyInputVideoInfo(cscSettingDetails);
            return cscSettingDetails;
        };
    _cscCore = PipelineCore::Create(_spCsc, cscCorePipelineHandler);

    return true;
}

bool UdxVvpIspPipeline::InitOutputMixerTpg()
{
    // TODO : Move these IDs to configuration provider class.
    // (reads from default -> JSON -> command line)
    const uint32_t MIXER_TPG_UID = PipelineBaseUID(PipelineSubsystem::Output);

    auto spVvpMixerTpgInst = _spHapi->CreateByUniqueID<Hapi::VvpTpg>(MIXER_TPG_UID);
    if (!spVvpMixerTpgInst)
    {
        return false;
    }

    _spOutputMixerTpg = ITpg::Create(spVvpMixerTpgInst, 1920, 1080);
    if (!_spOutputMixerTpg)
    {
        return false;
    }

    _spOutputMixerTpg->SetOutputWidth(3840);
    _spOutputMixerTpg->SetOutputHeight(2160);
    _spOutputMixerTpg->SetColorsForUniformPattern(0x0, 0x0, 0x0);
	// The Mixer TPG needs to be activated last
    _spOutputMixerTpg->SetEnable(true);

    // Pipeline core

    auto outputMixerTpgCorePipelineHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_outputMixerTpgCore->_outputCores.size() == 1);
            ASSERT(_outputMixerTpgCore->_inputCores.size() == 0, "The VVP TPG does not take input - it's an output-only core.");

            _spOutputMixerTpg->SetOutputWidth(inputDetails._width);
            _spOutputMixerTpg->SetOutputHeight(inputDetails._height);

            TRACE << "[Output Mixer TPG]: " << LOG_V(inputDetails) << "\n" << std::flush;

            _outputMixerTpgCore->_currentOutputVideoDetails = inputDetails;

            return inputDetails;
        };
    _outputMixerTpgCore = PipelineCore::Create(_spOutputMixerTpg, nullptr, outputMixerTpgCorePipelineHandler);

    return true;
}

bool UdxVvpIspPipeline::InitOverlayVfr()
{
    uint32_t primary_width = 0;
    uint32_t primary_height = 0;
    uint32_t overlay_width = 0;
    uint32_t overlay_height = 0;

    _drmHelper = IDrmHelper::GetIDrmHelper();

	/* open the DRM device */
    const char* card = "/dev/dri/card0";

#ifndef ISP_AI_BUILD
    primary_width = 0;
    primary_height = 0;
    lv_color_format_t primary_format = LV_COLOR_FORMAT_UNKNOWN;

#else
    primary_width = 960;
    primary_height = 540;
    lv_color_format_t primary_format = LV_COLOR_FORMAT_ARGB2222;
#endif
	if(_drmHelper->Open(card, primary_width, primary_height, primary_format))
    {
        primary_height = _drmHelper->GetPrimaryHeight();
        primary_width = _drmHelper->GetPrimaryWidth();
        uint32_t primary_stride = _drmHelper->GetPrimaryStride();
        uint8_t * primary_fb_ptr = _drmHelper->GetPrimaryBuffer();
        memset(primary_fb_ptr, 0, primary_height*primary_stride);

        overlay_height = _drmHelper->GetOverlayHeight();
        overlay_width = _drmHelper->GetOverlayWidth();
        uint32_t overlay_stride = _drmHelper->GetOverlayStride();
        uint8_t * overlay_fb_ptr = _drmHelper->GetOverlayBuffer();

        (void)overlay_width; // Unused, kept for debugging purposes

        memset(overlay_fb_ptr, 0, overlay_height*overlay_stride);
    }
    IUIConnection* uiConnection = static_cast<IUIConnection*>(VvpIspDemo::Get());
    auto weak = weak_from_this();
    _lvglLogoHelper = std::make_shared<lvglLogoHelper>(weak, uiConnection);

    _overlay_mixer_width = 3840;
    _overlay_mixer_height = 2160;

    const uint32_t OVERLAY_SCALER_UID = PipelineBaseUID(PipelineSubsystem::Output);
    auto spVvpOverlayScalerInst = _spHapi->CreateByUniqueID<Hapi::VvpScaler>(OVERLAY_SCALER_UID);
    if (!spVvpOverlayScalerInst)
    {
        TRACE
            << "Overlay Scaler not present\n" << std::flush;
    }
    else
    {
        _spOverlayScaler = Scaler::Create(spVvpOverlayScalerInst);
        if (!_spOverlayScaler)
        {
            TRACE
                << "Failed to init overlay Scaler\n" << std::flush;
            return false;
        }
        _spOverlayScaler->SetCoefficientBanks(0, 0);
        _spOverlayScaler->SetInputResolution(primary_width,primary_height);
        _spOverlayScaler->SetOutputResolution(_overlay_mixer_width,_overlay_mixer_height);
        _spOverlayScaler->Commit();
    }

    return true;
}


bool UdxVvpIspPipeline::InitOutputMixer()
{
    const uint32_t MIXER_UID = PipelineBaseUID(PipelineSubsystem::Output);
    auto spVvpMixerInst = _spHapi->CreateByUniqueID<Hapi::VvpMixer>(MIXER_UID);
    if (!spVvpMixerInst) return false;

    auto spSwApiMixer = IMixer::Create(spVvpMixerInst);
    if (!spSwApiMixer) return false;
    
    _spOutputMixer = std::make_shared<IspMixerHelper>(spSwApiMixer, (_spOverlayScaler != nullptr), true);
    if (!_spOutputMixer) return false;    

    ::intel_vvp_core_set_img_info_subsampling(spVvpMixerInst->GetInstance(), 0x3);

    auto outputMixerCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_outputMixerCore->_outputCores.size() == 1);

            ImageConfig inputDetails = outputDetails;
            _spOutputMixer->SetOutputResolution(outputDetails._width, outputDetails._height);

            if(_spOverlayScaler)
            {
                _overlay_mixer_width = outputDetails._width;
                _overlay_mixer_height = outputDetails._height;
                _spOverlayScaler->SetOutputResolution(_overlay_mixer_width,_overlay_mixer_height);
                _spOverlayScaler->Commit();
            }

            _spOutputMixer->SetIspResolution(inputDetails._width, inputDetails._height);

            _outputMixerCore->_currentInputVideoDetails = inputDetails;
            _outputMixerCore->_currentOutputVideoDetails = outputDetails;

            TRACE << "[Output Mixer]: " << LOG_V(outputDetails) << '\n' << std::flush;

            for(uint input_idx = 0U; input_idx < _outputMixerCore->_inputCores.size(); ++input_idx)
            {
                _outputMixerCore->_inputCores[input_idx]->ApplyOutputVideoInfo(inputDetails);
            }

            return inputDetails;
        };
        
    _outputMixerCore = PipelineCore::Create(_spOutputMixer, nullptr, outputMixerCorePipelineOutputHandler);

    _spOutputMixer->EnableOverlay(true);

    return true;
}


bool UdxVvpIspPipeline::InitUsm()
{
    const uint32_t USM_UID = PipelineBaseUID(PipelineSubsystem::PostCoreISP);
    _spUsm = _spHapi->CreateByUniqueID<Hapi::VvpUsm>(USM_UID);
    if (!_spUsm) return false;

    auto _spVvpUsmInst = _spUsm->GetInstance();

    int err = intel_vvp_usm_init(_spVvpUsmInst, _spVvpUsmInst->core_instance.base);
    if (err != kIntelVvpCoreOk )
    {
        TRACE << "Failed to intialise USM instance\n" << std::flush;
        return false;
    }

    err = intel_vvp_usm_set_sharpening_strength(_spVvpUsmInst, 0);
    if (err != kIntelVvpCoreOk )
    {
        TRACE << "Failed to set initial USM strength\n" << std::flush;
        return false;
    }

    if (!intel_vvp_usm_set_output_width(_spVvpUsmInst, 1920u))
    {
        TRACE
            << "Failed to set initial USM frame width\n" << std::flush;
        return false;
    }

    if (!intel_vvp_usm_set_output_height(_spVvpUsmInst, 1080u))
    {
        TRACE << "Failed to set initial USM frame height\n" << std::flush;
        return false;
    }

    // Pipeline core
    auto usmCorePipelineInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_usmCore->_inputCores.size() == 1 && _usmCore->_outputCores.size() == 1);

            TRACE << "[USM]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

            intel_vvp_usm_set_output_width(_spUsm->GetInstance(), inputDetails._width);
            intel_vvp_usm_set_output_height(_spUsm->GetInstance(), inputDetails._height);

            _usmCore->_currentInputVideoDetails = inputDetails;
            _usmCore->_currentOutputVideoDetails = inputDetails;
            _usmCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    auto usmCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_usmCore->_inputCores.size() == 1 && _usmCore->_outputCores.size() == 1);

            TRACE << "[USM]:\t" << LOG_V(outputDetails) << '\n' << std::flush;

            intel_vvp_usm_set_output_width(_spUsm->GetInstance(), outputDetails._width);
            intel_vvp_usm_set_output_height(_spUsm->GetInstance(), outputDetails._height);

            _usmCore->_currentInputVideoDetails = outputDetails;
            _usmCore->_currentOutputVideoDetails = outputDetails;
            _usmCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);

            return outputDetails;
        };
    _usmCore = PipelineCore::Create(_spUsm, usmCorePipelineInputHandler, usmCorePipelineOutputHandler);

#ifndef ISP_STITCH_BUILD
    _coreIspPipelines[0]->InsertAdditionalCore(_usmCore);
#else
    _postCoreIspInputStack.push_back(_usmCore);
#endif

    return true;
}


bool UdxVvpIspPipeline::InitTmo()
{
    const uint32_t TMO_UID = PipelineBaseUID(PipelineSubsystem::PostCoreISP);
    auto spVvpTmo = _spHapi->CreateByUniqueID<Hapi::VvpTmo>(TMO_UID);
    if (!spVvpTmo)
    {
        TRACE << "Failed to get TMO hapi core\n" << std::flush;
        return false;
    }

    _spTmoBase = ITmoBase::Create(spVvpTmo, 1920u, 1080u);
    if (!_spTmoBase)
    {
        TRACE << "Failed to create TMO interface\n" << std::flush;
        return false;
    }

    _spTmoBase->SetResolution(1920u, 1080u);
    _spTmoBase->SetOutputWidth(1920u);
    _spTmoBase->SetOutputHeight(1080u);

    // Pipeline core

    auto tmoCorePipelineInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_tmoCore->_inputCores.size() == 1 && _tmoCore->_outputCores.size() == 1);

            _spTmoBase->SetResolution(inputDetails._width, inputDetails._height);

            _spTmoBase->SetOutputWidth(inputDetails._width);
            _spTmoBase->SetOutputHeight(inputDetails._height);

            _tmoCore->_currentInputVideoDetails = inputDetails;
            _tmoCore->_currentOutputVideoDetails = inputDetails;
            // This is hardcoded for now but should be retrieved from the core
            _tmoCore->_currentOutputVideoDetails.SetColourDepth(TColourDepth::BPC10);

            TRACE << "[TMO]:\t" << LOG_IO(inputDetails,  _tmoCore->_currentOutputVideoDetails) << '\n' << std::flush;

            _tmoCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    auto tmoCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_tmoCore->_inputCores.size() == 1 && _tmoCore->_outputCores.size() == 1);

            _spTmoBase->SetResolution(outputDetails._width, outputDetails._height);

            _spTmoBase->SetOutputWidth(outputDetails._width);
            _spTmoBase->SetOutputHeight(outputDetails._height);

            _tmoCore->_currentInputVideoDetails = outputDetails;
            _tmoCore->_currentOutputVideoDetails = outputDetails;
            // This is hardcoded for now but should be retrieved from the core
            _tmoCore->_currentOutputVideoDetails.SetColourDepth(TColourDepth::BPC10);

            TRACE << "[TMO]:\t" << LOG_V(outputDetails) << '\n' << std::flush;

            _tmoCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);

            return outputDetails;
        };
    _tmoCore = PipelineCore::Create(_spTmoBase, tmoCorePipelineInputHandler, tmoCorePipelineOutputHandler);

#ifndef ISP_STITCH_BUILD
    _coreIspPipelines[0]->InsertAdditionalCore(_tmoCore);
#else
    _postCoreIspInputStack.push_back(_tmoCore);
#endif

    return true;
}

bool UdxVvpIspPipeline::InitLut3d()
{    
    const uint32_t LUT3D_UID = PipelineBaseUID(PipelineSubsystem::PostCoreISP);
    _spVvp3dLut = _spHapi->CreateByUniqueID<Hapi::Vvp3dLut>(LUT3D_UID);
    if (!_spVvp3dLut)
    {
        TRACE << "Failed to get 3D LUT hapi core\n" << std::flush;
        return false;
    }

    _spLut3d = SwApi::ILut3d::Create(_spVvp3dLut);
    if (!_spLut3d)
    {
        TRACE << "Failed to create 3D LUT interface\n" << std::flush;
        return false;
    }

    // Pipeline core

    auto lut3dCorePipelineInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_lut3dCore->_inputCores.size() == 1 && _lut3dCore->_outputCores.size() == 1);

            TRACE << "[3D LUT]:\t" << LOG_V(inputDetails) << '\n';

            ::intel_vvp_core_set_img_info_height(_spVvp3dLut->GetInstance(),
                                                inputDetails._height);
            ::intel_vvp_core_set_img_info_width(_spVvp3dLut->GetInstance(),
                                                inputDetails._width);

            _lut3dCore->_currentInputVideoDetails = inputDetails;
            _lut3dCore->_currentOutputVideoDetails = inputDetails;
            _lut3dCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    auto lut3dCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
            ASSERT(_lut3dCore->_inputCores.size() == 1 && _lut3dCore->_outputCores.size() == 1);

            TRACE << "[3D LUT]:\t" << LOG_V(outputDetails) << '\n';

            ::intel_vvp_core_set_img_info_height(_spVvp3dLut->GetInstance(),
                                                outputDetails._height);
            ::intel_vvp_core_set_img_info_width(_spVvp3dLut->GetInstance(),
                                                outputDetails._width);

            _lut3dCore->_currentInputVideoDetails = outputDetails;
            _lut3dCore->_currentOutputVideoDetails = outputDetails;
            _lut3dCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);

            return outputDetails;
        };
    _lut3dCore = PipelineCore::Create(_spLut3d, lut3dCorePipelineInputHandler, lut3dCorePipelineOutputHandler);

#ifndef ISP_STITCH_BUILD
    _coreIspPipelines[0]->InsertAdditionalCore(_lut3dCore);
#else
    _postCoreIspInputStack.push_back(_lut3dCore);
#endif

    return true;
}


bool UdxVvpIspPipeline::InitHdrComponents()
{
    // 1D LUT
    const uint32_t HDR_LUT1D_UID = PipelineBaseUID(PipelineSubsystem::PostCoreISP);
    auto spVvpLut1d = _spHapi->CreateByUniqueID<Hapi::Vvp1dLut>(HDR_LUT1D_UID);

    if (!spVvpLut1d)
        return false;

    auto spVvpLut1dInst = spVvpLut1d->GetInstance();

    if (intel_vvp_1d_lut_init(spVvpLut1dInst, spVvpLut1dInst->core_instance.base) != kIntelVvpCoreOk ){
        TRACE << "Failed to intialise instance of 1D LUT Linear -> RGB\n" << std::flush;

        return false;
    }

    _spLut1dLinearSrgb = SwApi::Lut1D::Create(spVvpLut1d);

    if (!_spLut1dLinearSrgb)
        return false;

    if(!_spLut1dLinearSrgb->SetResolution(1920u, 1080u))
    {
        TRACE << "Failed to set 1D LUT Linear -> RG output resolution\n" << std::flush;
        return false;
    }

    // 1D LUT Pipeline Core

    auto lut1dLinearSrgbCorePipelineHandler = [this](const ImageConfig& inputDetails) -> ImageConfig
    {
        const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
        ASSERT(_lut1dLinearSrgbCore->_inputCores.size() == 1);
        ASSERT(_lut1dLinearSrgbCore->_outputCores.size() == 1);

        TRACE << "[1D LUT Linear->RGB]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

        _spLut1dLinearSrgb->SetResolution(inputDetails._width, inputDetails._height);

        _lut1dLinearSrgbCore->_currentInputVideoDetails = inputDetails;
        _lut1dLinearSrgbCore->_currentOutputVideoDetails = inputDetails;

        _lut1dLinearSrgbCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

        return inputDetails;
    };

    _lut1dLinearSrgbCore = PipelineCore::Create(_spLut1dLinearSrgb, lut1dLinearSrgbCorePipelineHandler);

#ifndef ISP_STITCH_BUILD
    _coreIspPipelines[0]->InsertAdditionalCore(_lut1dLinearSrgbCore);
#else
    _postCoreIspInputStack.push_back(_lut1dLinearSrgbCore);
#endif

    // 3D LUT

    const uint32_t HDR_LUT3D_UID = PipelineBaseUID(PipelineSubsystem::PostCoreISP) + 1;
    auto spVvp3dLut = _spHapi->CreateByUniqueID<Hapi::Vvp3dLut>(HDR_LUT3D_UID);

    if (!spVvp3dLut)
        return false;

    _spLut3dSrgbHlg = SwApi::ILut3d::Create(spVvp3dLut);

    if (!_spLut3dSrgbHlg)
    {
        TRACE << "Failed to create sRGB -> HLG 3D LUT interface\n" << std::flush;
        return false;
    }

    // 3D LUT Pipeline core

    auto lut3dCorePipelineHandler = [this, spVvp3dLut](const ImageConfig& inputDetails) -> ImageConfig {
        const std::lock_guard<std::recursive_mutex> lock(_mutex);
            
        ASSERT(_lut3dSrgbHlgCore->_inputCores.size() == 1 && _lut3dSrgbHlgCore->_outputCores.size() == 1);

        TRACE << "[3D LUT sRGB->HLG]:\t" << LOG_IO(inputDetails, inputDetails) << '\n' << std::flush;

        ::intel_vvp_core_set_img_info_height(spVvp3dLut->GetInstance(),
                                            inputDetails._height);
        ::intel_vvp_core_set_img_info_width(spVvp3dLut->GetInstance(),
                                            inputDetails._width);

        _lut3dSrgbHlgCore->_currentInputVideoDetails = inputDetails;
        _lut3dSrgbHlgCore->_currentOutputVideoDetails = inputDetails;
        _lut3dSrgbHlgCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

        return inputDetails;
    };

    _lut3dSrgbHlgCore = PipelineCore::Create(_spLut3dSrgbHlg, lut3dCorePipelineHandler);

#ifndef ISP_STITCH_BUILD
    _coreIspPipelines[0]->InsertAdditionalCore(_lut3dSrgbHlgCore);
#else
    _postCoreIspInputStack.push_back(_lut3dSrgbHlgCore);
#endif

    return true;
}


void UdxVvpIspPipeline::LinkPipelineCores()
{
    _tpgCore->ConnectOutput(_bayerSwitchCore);

    // Link cameara inputs
    for(std::size_t i = 0; i < _sensorIntfCore.size(); ++i)
    {
        const auto inputPipConv = (_inputPipConvCore.size() > i) ? _inputPipConvCore[i] : nullptr;

        const auto output_core = inputPipConv ? _inputPipConvCore[i] : _bayerSwitchCore;

        if(_exposureFusionCore[i])
        {
            _sensorIntfCore[i]->ConnectOutput(_exposureFusionCore[i]);
            _exposureFusionCore[i]->ConnectOutput(std::move(output_core));
        }
        else
        {
            _sensorIntfCore[i]->ConnectOutput(std::move(output_core));
        }

        if(inputPipConv)
           inputPipConv->ConnectOutput(_bayerSwitchCore);
    }

    if(_vfrInputCore)
        _vfrInputCore->ConnectOutput(_bayerSwitchCore);    

    for(auto coreIspPipeline : _coreIspPipelines)
    {
        coreIspPipeline->LinkCoreIspPipelineCores();
    }

    // Now build the ISP chain. Function order previously is
#ifdef ISP_STITCH_BUILD
    for(uint32_t pipeline_index = 0; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
    {
        auto& coreIspPipeline = _coreIspPipelines[pipeline_index];
        coreIspPipeline->GetInputCore()->ConnectInput(_bayerSwitchCore);
    }
#else
    _coreIspPipelines[0]->GetInputCore()->ConnectInput(_bayerSwitchCore);
#endif

    // EXTREMELY important here.
    auto lastCore = _outputMixerCore;
    while (_postCoreIspInputStack.size() > 0)
    {
        auto core = _postCoreIspInputStack.back();
        lastCore->ConnectInput(core);
        lastCore = std::move(core);
        _postCoreIspInputStack.pop_back();
    };
#ifndef ISP_STITCH_BUILD
    lastCore->ConnectInput(_coreIspPipelines[0]->GetOutputCore());
#endif

    // ISP output
    if(_outputMixerCore)
    {
        _outputMixerCore->ConnectInput(_outputMixerTpgCore);
    }

    if(_outputMixerCore)
    {
        _terminalCore = _outputMixerCore;
    }

    if(_lut1dCore)
    {
        _terminalCore->ConnectOutput(_lut1dCore);
        _terminalCore = _lut1dCore;
    }

    if(_outputPipConvCore)
    {
        _terminalCore->ConnectOutput(_outputPipConvCore);
        _terminalCore = _outputPipConvCore;
    }

    if(_protoConvLTFDpTxCore)
    {
        _terminalCore->ConnectOutput(_protoConvLTFDpTxCore);
        _terminalCore = _protoConvLTFDpTxCore;
    }
}


void UdxVvpIspPipeline::SetOverlayResolution(uint32_t width, uint32_t height)
{
    _drmHelper->SetOverlayResolution(width, height);
    _spOutputMixer->SetLogoResolution(width, height);
}

bool UdxVvpIspPipeline::ValidateVideoStandard(const VideoStandard& vs)
{
    const auto check_resolution = [](const VideoStandard& vs)->bool {
        static constexpr uint32_t supported_resolutions[][2] = {
            {3840, 2160},
            {1920, 1080}
        };

        const auto it = std::ranges::find_if(supported_resolutions, [&vs](const auto& r){
            return (vs.Width() == r[0] && vs.Height() == r[1]);
        });

        return it != std::end(supported_resolutions);
    };

    return check_resolution(vs);
}


void UdxVvpIspPipeline::WatchInput()
{
    bool inputVideoStandardChanged = false;
    for(std::size_t idx = 0; idx < _inputs.size(); ++idx)
    {
        VideoStandard vs{};
        vs.SetInterlaced(false);
        const auto& input = _inputs[idx];

        for(uint32_t pipeline_index = 0; pipeline_index < _coreIspPipelines.size(); pipeline_index++)
        {
            switch(input)
            {
                case PipelineInput::Camera0:
                case PipelineInput::Camera1:
                {
                    const uint32_t cam_idx = (input == PipelineInput::Camera0 ? 0 : 1);
                    const auto& cam = _cameras[cam_idx];
                    const uint32_t mipi_interface = cam->GetMIPIInterface();
                    auto inputSnoop = _spInputSnoop[mipi_interface];

                    if(inputSnoop)
                    {
                        auto instance = inputSnoop->GetInstance();
                        const uint32_t width = intel_vvp_snoop_get_last_max_width(instance);
                        const uint32_t height = intel_vvp_snoop_get_last_num_lines(instance);

                        vs.SetWidth(width + 1);
                        vs.SetHeight(height);
                    }
                    else
                    {
                        const auto [width, height] = cam->GetResolution();
                        vs.SetWidth(width);
                        vs.SetHeight(height);
                    }

                    vs.SetColourDepth(TColourDepth::BPC12);
                    vs.SetFrameRate(static_cast<uint32_t>(cam->GetFrameRate() * 100.0f));
                }
                break;
                case PipelineInput::Cameras:
                {
                    for(uint32_t cam_idx = 0 ; cam_idx < _cameras.size(); cam_idx++)
                    {
                        const auto& cam = _cameras[cam_idx];
                        const uint32_t mipi_interface = cam->GetMIPIInterface();
                        auto inputSnoop = _spInputSnoop[mipi_interface];

                        if(inputSnoop)
                        {
                            auto instance = inputSnoop->GetInstance();
                            const uint32_t width = intel_vvp_snoop_get_last_max_width(instance);
                            const uint32_t height = intel_vvp_snoop_get_last_num_lines(instance);

                            if(cam_idx == 0)
                            {
                                vs.SetWidth(width + 1);
                                vs.SetHeight(height);
                            }
                            else
                            {
                                if((vs.Width() != width + 1) || (vs.Height() != height))
                                {
                                    vs.SetWidth(0);
                                    vs.SetHeight(0);
                                }
                            }
                        }
                        else
                        {
                            const auto [width, height] = cam->GetResolution();
                            if(cam_idx == 0)
                            {
                                vs.SetWidth(width);
                                vs.SetHeight(height);
                            }
                            else
                            {
                                if((vs.Width() != width) || (vs.Height() != height))
                                {
                                    vs.SetWidth(0);
                                    vs.SetHeight(0);
                                }
                            }
                        }

                        vs.SetColourDepth(TColourDepth::BPC12);
                        vs.SetFrameRate(cam->GetTargetFrameRate());
                    }
                }
                break;
                case PipelineInput::TPG:
                {
                    const uint32_t width = _spTpg->GetOutputWidth();
                    const uint32_t height = _spTpg->GetOutputHeight();
                    vs.SetWidth(width);
                    vs.SetHeight(height);
                    vs.SetColourDepth(TColourDepth::BPC16); // ToDo: Set depending on the pipeline width
                    vs.SetFrameRate(_framerate);

                }
                break;
                case PipelineInput::FR:
                {
                    if(_spVfrInput)
                    {
                        const auto [width, height] = _spVfrInput->GetResolution();
                        vs.SetWidth(width);
                        vs.SetHeight(height);
                        vs.SetColourDepth(TColourDepth::BPC16);
                        vs.SetFrameRate(_framerate);
                    }
                }
                break;
                default:
                break;
            }

            if(input != PipelineInput::FR)
            {
                if(!ValidateVideoStandard(vs))
                    vs = VideoStandard{};
            }

            if(_spInputState->_inputVideoStandard[input] != vs)
            {
                const auto inputName = InputToString(input);

                if(_coreIspPipelines.size() > 1)
                {
                    TRACE << "[" << inputName << "]\t pipeline " << pipeline_index << " new video standard: " << vs.ToString() << '\n' << std::flush;
                }
                else
                {
                    TRACE << "[" << inputName << "]\t new video standard: " << vs.ToString() << '\n' << std::flush;
                }

                _spInputState->_inputVideoStandard[input] = vs;
                _spInputState->_inputVideoStandardChanged = true;
            }

            if(_spInputState->_inputVideoStandardChanged)
            {
                inputVideoStandardChanged = true;
            }
        }
    }
    if (inputVideoStandardChanged)
    {
        ReprogramPipeAction(PipelineStateChanged::PreWarp);
    }
}


// Watch for the output change here
bool UdxVvpIspPipeline::WatchOutputTask()
{
    return WatchOutputDp();
}


std::string  UdxVvpIspPipeline::InputToString(const PipelineInput& input) const
{
    static const std::map<PipelineInput, std::string> input_names = {
        {PipelineInput::TPG,        "TPG"},
        {PipelineInput::Camera0,    "Camera 0"},
        {PipelineInput::Camera1,    "Camera 1"},
        {PipelineInput::Cameras,    "Cameras"},
        {PipelineInput::FR,         "Frame Reader"},
        {PipelineInput::Invalid,    "Unknown"}
    };

    const auto input_name = input_names.find(input);
    return (input_name != input_names.end() ? input_name->second : "Unknown");
}

uint32_t UdxVvpIspPipeline::PipelineBaseUID(PipelineSubsystem subsystem, uint32_t pipeline_index)
{
    uint32_t baseUID;

    switch(subsystem)
    {
    case PipelineSubsystem::Camera:
        baseUID = 10U + pipeline_index;
        break;
    case PipelineSubsystem::Input:
        baseUID = 20U;
        break;
    case PipelineSubsystem::CoreISP:
#ifndef ISP_STITCH_BUILD
        baseUID = 30U;
#else
        baseUID = 60U + 10U*pipeline_index;
#endif
        break;
    case PipelineSubsystem::PostCoreISP:
#ifndef ISP_STITCH_BUILD
        baseUID = 30U;
#else
        baseUID = 80U;
#endif
        break;
    case PipelineSubsystem::Output:
        baseUID = 50U;
        break;
    case PipelineSubsystem::Misc:
        baseUID = 10U;
        break;
    default:
        baseUID = 0U;
        break;
    }
    
    return baseUID;
}

////////////// DP TX Multirate support ////////////////

void UdxVvpIspPipeline::SetOutputVideoStandard(const VideoStandard& vs)
{
    if(_spOutputState->_outputVideoStandard != vs)
    {
        _spOutputState->_outputVideoStandard = vs;
        ReprogramPipeAction(PipelineStateChanged::PostMixer);
    }
}


void UdxVvpIspPipeline::RequestOutputVideoStandardOverride(const uint32_t idx, const bool bpc10)
{
    uint32_t reg_val = 0x0;

    std::lock_guard lock{_mutex};
    
    if(idx > 0)
    {
        // idx is 1-based, 0 is "No override"
        reg_val = (0x1 << (idx - 1)) | (bpc10 ? FORMATS_REG_10BPC : 0x0);
    }

    intel_pio_write(_dpTxOverridePio->GetInstance(), reg_val);
    _dpTxOverrideActive = (reg_val != 0x0);
}


bool UdxVvpIspPipeline::WatchOutputDp()
{
    bool ret = false;
    static uint32_t dptx_status = 0x0;
    static uint32_t dptx_format = 0x0;
    static uint32_t dptx_opts = 0x0;
    static const video_format_t* dp_format{nullptr};

#ifdef MAX_30_FPS
    static const uint32_t framerate_limit = 3000;
#else
    static const uint32_t framerate_limit = 0;
#endif

    uint32_t dptx_format_current = intel_pio_read(_dpTxActivDimPio->GetInstance());
    uint32_t dptx_opts_current = intel_pio_read(_dpTxOverrideOptsPio->GetInstance());
    uint32_t dptx_status_current = intel_pio_read(_dpTxStatusPio->GetInstance());
    
    if((dptx_status_current != dptx_status) || (dptx_format_current != dptx_format) || (dptx_opts_current != dptx_opts))
    {
        dptx_format = dptx_format_current;
        dptx_opts = dptx_opts_current;
        dptx_status = dptx_status_current;

        uint32_t v = dptx_format_current & FORMATS_REG_MASK;
        uint32_t idx = 0;

        while((v & 0x1) == 0)
        {
            ++idx;
            v = v >> 1;
        }

        dp_format = dptx_formats_get(idx);

        if(dp_format)
        {
            const auto bpc = (dptx_format & FORMATS_REG_10BPC) ? TColourDepth::BPC10 : TColourDepth::BPC8;
            TColourInfo ci = {TColourSpace::RGB, bpc, 0, 0};
            VideoStandard vs = VideoStandard{3840, 2160, 6000, ci};
            vs.SetWidth(dp_format->timing.sample_count);
            vs.SetHeight(dp_format->timing.f0_line_count);
            vs.SetFrameRate(dp_format->fps);

            const uint32_t output_full_height = dp_format->timing.f0_line_count + dp_format->timing.f1_line_count + dp_format->timing.v_blanking;

            auto updateVsCmd = [this, vs = std::move(vs), output_full_height](){
                _spOutputState->_outputFullHeight = output_full_height;
                SetOutputVideoStandard(vs);
            };

            VvpIspDemo::Get()->AddCommand(updateVsCmd);
        }

        std::string dp_format_str{dp_format ? dp_format->str : "N/A"};

        if(dp_format)
        {
            dp_format_str += std::format(" {}bps", dptx_format & FORMATS_REG_10BPC ? 10 : 8);
        }

        std::vector<uint32_t> dptx_ui_opts{};

        const uint32_t num_formats = std::min(static_cast<uint32_t>(MAX_SUPPORTED_FORMATS), dptx_formats_len());

        for(uint32_t i = 0; i < num_formats; ++i)
        {
            if(dptx_opts & (0x1 << i))
                dptx_ui_opts.push_back(i);
        }

        const bool bpc10_support = dptx_opts & FORMATS_REG_10BPC;

        auto updateUiCmd = [dp_format_str = std::move(dp_format_str), dptx_ui_opts = std::move(dptx_ui_opts), bpc10_support](){
            VvpIspDemo::Get()->GetUi()->UpdateOutputStatus(dptx_status, dp_format_str, dptx_ui_opts, bpc10_support);
        };

        VvpIspDemo::Get()->AddCommand(updateUiCmd);
    }

    // Nios DP software picks the best available video standard for the current sink
    // We might want to use one with a lower frame rate based on the video pipeline ability
    // The code below cheks and overrides the current format if necessary
    if(dp_format && (framerate_limit > 0))
    {
        if(dp_format->fps > framerate_limit)
        {
            std::lock_guard lock{_mutex};

            if(!_dpTxOverrideActive)
            {
                std::size_t dp_fmt_override = 0;

                const uint32_t num_formats = std::min(static_cast<uint32_t>(MAX_SUPPORTED_FORMATS), dptx_formats_len());

                for(uint32_t i = 0; i < num_formats; ++i)
                {
                    if(dptx_opts & (0x1 << i))
                    {
                        const auto fmt = dptx_formats_get(i);

                        if(fmt && (fmt->fps <= framerate_limit))
                        {
                            // Override value is 1-based, 0 is "No override"
                            dp_fmt_override = i + 1;
                            break;
                        }
                    }
                }

                if(0 == dp_fmt_override)
                    dp_fmt_override = 6; // Default to 1080p30 if nothing else found

                if(dp_fmt_override)
                {
                    const bool bpc10_support = dptx_opts & FORMATS_REG_10BPC;
                    RequestOutputVideoStandardOverride(dp_fmt_override, bpc10_support);
                }
            }
        }
    }

    return ret;
}


bool UdxVvpIspPipeline::InitDpTxMultirate()
{
    bool ret = false;

    const uint32_t DPTX_STATUS_PIO_ID = PipelineBaseUID(PipelineSubsystem::Output) + 1U;
    const uint32_t DPTX_OVERRIDE_OPTS_PIO_ID = PipelineBaseUID(PipelineSubsystem::Output) + 2U;
    const uint32_t DPTX_ACTIV_DIM_PIO_ID = PipelineBaseUID(PipelineSubsystem::Output) + 3U;
    const uint32_t DPTX_FORMAT_PIO_ID = PipelineBaseUID(PipelineSubsystem::Output) + 4U;

    _dpTxActivDimPio = _spHapi->CreateByUniqueID<Hapi::Pio>(DPTX_ACTIV_DIM_PIO_ID);
    _dpTxOverrideOptsPio = _spHapi->CreateByUniqueID<Hapi::Pio>(DPTX_OVERRIDE_OPTS_PIO_ID);
    _dpTxOverridePio = _spHapi->CreateByUniqueID<Hapi::Pio>(DPTX_FORMAT_PIO_ID);
    _dpTxStatusPio = _spHapi->CreateByUniqueID<Hapi::Pio>(DPTX_STATUS_PIO_ID);
    

    if(_dpTxActivDimPio && _dpTxOverrideOptsPio && _dpTxOverridePio && _dpTxStatusPio)
    {
        // No override by default
        intel_pio_write(_dpTxOverridePio->GetInstance(), 0x0);

        _dpTxMultirateEnabled = true;

        ret = true;
    }

    return ret;
}


SwApi::vfw_frame_t UdxVvpIspPipeline::CaptureRawFrame(const uint32_t targetBps, bool RGB)
{
    _spVfwCaptureRaw->CaptureFrame();  

    auto frame = _spVfwCaptureRaw->ReadFrame(16, true);

    return frame;
}


SwApi::vfw_frame_t UdxVvpIspPipeline::CaptureProcessedFrame(const uint32_t targetBps, bool RGB)
{
    _spVfwCaptureIsp->CaptureFrame();
    auto frame = _spVfwCaptureIsp->ReadFrame(16, true);

    return frame;
}


PipelineInputState::PipelineInputState():
    _input{PipelineInput::Invalid}
{
}


std::shared_ptr<PipelineInputState> PipelineInputState::Clone()
{
    auto spClone = std::make_shared<PipelineInputState>();
    *spClone = *this;
    return spClone;
}

PipelineOutputState::PipelineOutputState()
{
}


std::shared_ptr<PipelineOutputState> PipelineOutputState::Clone()
{
    auto spClone = std::make_shared<PipelineOutputState>();
    *spClone = *this;
    return spClone;
}

