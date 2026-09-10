/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "system.h"

#include "AtUtils.h"
#include "CommonCommands.h"
#include "CommandLine.h"

#include "VideoStandard.h"
#include "PipelineCore.h"
#include "ICamera.h"
#include "IFrameCapture.h"
#include "drmHelper.h"
#include "lvglLogoHelper.h"
#include "ILogoControl.h"

#include "CoreIspPipeline.h"

// Interfaces
#include "HapiVvpExposureFusion.h"
#include "HapiVvpPipConv.h"
#include "ITpg.h"
#include "ICrs.h"
#include "ISwitch.h"
#include "HapiVvpUsm.h" //USM has no SwApi so it needs to be included here
#include "Scaler.h"
#include "WarpAdapter.h"
#include "ITmo.h"
#include "ILut3d.h"
#include "IMixer.h"
#include "IspMixerHelper.h"
#include "Lut1D.h"
#include "HapiVvpVfw.h"
#include "VideoFrameReader.h"
#include "HapiVvpAlphaChannelGenerator.h"
#include "IspVfrInput.h"
#include "HapiVvpROI.h"
#include "ROIHelper.h"
#include "VideoFrameWriter.h"
#include "HapiAddrSpanExpander.h"
#include "HapiVvpCpm.h"
#include "HapiVvpSnoop.h"
#include "CalibrationProfile.h"
#include "Remosaic.h"
#include "HapiPio.h"
#include "HapiVvpProtocolConv.h"

#include <deque>
#include <stack>
#include <map>


enum class PipelineInput
{
    Camera0,
    Camera1,
    Cameras,
    TPG,
    FR,
    Invalid
};


enum class OutputSource
{
    ISP,
    ColorBars,
    Invalid
};

class PipelineInputState 
{
public:
    PipelineInputState();
    std::shared_ptr<PipelineInputState> Clone();

    TCfaPhase _inputCfaPhase = TCfaPhase::RGGB;
    bool _inputVideoStandardChanged = true;

    PipelineInput _input;
    std::map<PipelineInput, VideoStandard> _inputVideoStandard;
};

class PipelineOutputState 
{
public:
    PipelineOutputState();
    std::shared_ptr<PipelineOutputState> Clone();

    VideoStandard _mixerOutput = {};
    bool _warpStateUpdated = true;
    bool _isOnWorkaroundState = true;
    bool _inputVideoStandardChanged = true;

    VideoStandard _outputVideoStandard = {};

    uint32_t _outputFrameRate = 60;
    uint32_t _outputFullHeight = 0;
};

class UdxVvpIspPipeline : public std::enable_shared_from_this<UdxVvpIspPipeline>, public IFrameCapture, public ILogoControl
{
public:
    enum class PipelineStateChanged
    {
        PreWarpNoClipperReset,
        PreWarp,
        PostWarp,
        PostMixer,
        ReprogramWholePipeline,
        Invalid
    };

    enum class PipelineSubsystem
    {
        Camera,
        Input,
        CoreISP,
        PostCoreISP,
        Output,
        Misc
    };

    UdxVvpIspPipeline();

    virtual ~UdxVvpIspPipeline();

    void Initialize(std::shared_ptr<Hapi::IHapi> spHapi);

    void LoadFuncsForUiControls(std::function<void(const ImageConfig&)> updateInputConfigCB,
                                std::function<void(const ImageConfig&)> updateOutputConfigCB);

    void LoadFuncsForUiFeedback(std::function<void(ImageConfig&)> warpInputSettingCB,
                                std::function<void(ImageConfig&)> mixerInputSettingCB,
                                std::function<void(ImageConfig&)> postPipelineProgramUiUpdateCB);

    bool Start();
    void Stop();

    void WarpEnabled(uint32_t pipeline_index, bool state);
    void ReprogramPipeAction(const PipelineStateChanged& stateChanged);

    const uint32_t GetNumberCoreIspPipelines() { return _coreIspPipelines.size();};

    // Input stage

    const std::shared_ptr<SwApi::ICrs>& GetCrs() { return _spCrs; }
    const std::shared_ptr<SwApi::ICsc>& GetCsc() { return _spCsc; }
    const std::shared_ptr<SwApi::ITpg>& GetTpg() { return _spTpg; }
    const std::shared_ptr<SwApi::ISwitch>& GetRgbSwitch() { return _spRgbSwitch; }

    const std::shared_ptr<SwApi::ISwitch>& GetBayerSwitch() { return _spBayerSwitch; }

    // ISP
    std::shared_ptr<CoreIspPipeline> GetCoreIspPipeline(uint32_t pipeline_index = 0) { return pipeline_index < _coreIspPipelines.size() ? _coreIspPipelines[pipeline_index] : nullptr; }
    const std::shared_ptr<Hapi::VvpUsm>& GetUsm() { return _spUsm; } 
   
    // Output
    const std::shared_ptr<SwApi::ITmoBase>& GetTmoBase() { return _spTmoBase; }
    const std::shared_ptr<SwApi::ILut3d>& GetLut3d() { return _spLut3d; }
    const std::shared_ptr<IspMixerHelper>& GetOutputMixer() { return _spOutputMixer; }
    const std::shared_ptr<lvglLogoHelper>& GetLogoHelper() { return _lvglLogoHelper; }
    const std::shared_ptr<SwApi::Lut1D>& GetLut1d() { return _spLut1d; }

    const std::shared_ptr<SwApi::VideoFrameWriter>& GetVfwCaptureRaw() {return _spVfwCaptureRaw; }
    const std::shared_ptr<SwApi::VideoFrameWriter>& GetVfwCaptureIsp() {return _spVfwCaptureIsp; }

    const std::shared_ptr<Hapi::VvpPipConv>& GetOutputPipConv() { return _spOutputPipConv; } 

    // HDR related
    const std::shared_ptr<SwApi::Lut1D>& GetLut1dLinearSrgb() { return _spLut1dLinearSrgb; }
    const std::shared_ptr<SwApi::ILut3d>& GetLut3dSrgbHlg() { return _spLut3dSrgbHlg; }

    void StartShutdown();

    const std::vector<ICameraPtr>& GetCameras() { return _cameras; }
    const std::vector<Hapi::VvpExposureFusionPtr>& GetExposureFusions() { return _spInputExposureFusion; };

    const std::shared_ptr<SwApi::IspVfrInput>& GetFrameReaderInput() { return _spVfrInput; };

    std::ostream& PrintUsage(std::ostream& out = std::cout);

    const std::vector<PipelineInput>& GetVideoInputs() const { return _inputs; }
    bool SelectInput(const uint32_t idx);
    PipelineInput GetUiSelectedInput();
    PipelineInput GetPipelineInputFromIndex(const uint32_t idx);

    bool SelectOutput(const uint32_t idx);

    void OnScreensaver(bool activate);

    void SetOutputVideoStandard(const VideoStandard& vs);
    void RequestOutputVideoStandardOverride(const uint32_t idx, const bool bpc10);

    virtual void SetOverlayResolution(uint32_t width, uint32_t height) override;

    virtual SwApi::vfw_frame_t CaptureRawFrame(const uint32_t targetBps, bool RGB) override;
    virtual SwApi::vfw_frame_t CaptureProcessedFrame(const uint32_t targetBps, bool RGB) override;

    bool IsActiveInputValid(uint32_t pipeline_index) const;
    void EnableIspMixerChannel(uint32_t pipeline_index, bool enable);

    void LinkPipelineCores();
    std::deque<std::shared_ptr<VideoPipeline::PipelineCore>>& GetPostCoreIspInputStack() { return _postCoreIspInputStack; }
    static uint32_t PipelineBaseUID(PipelineSubsystem subsystem, uint32_t pipeline_index=0U);

private:
    void ReadSettingsPIO();

    bool InitCrs();
    bool InitCsc();
    bool InitTpg();
    bool InitVfrInput();
    bool InitVfbSelectSwitches();
    bool InitCameras();
    bool InitInputPipConv(const uint32_t id);
    bool InitBayerSwitch();
    bool InitUsm();
    bool InitTmo();
    bool InitLut3d();
    bool InitScalers();
    bool InitLut1d();
    bool InitOutputPipConv();
    bool InitVfwCaptureRaw();
    bool InitVfwCaptureIsp();
    bool InitHdrComponents();
    bool InitOutputMixerTpg();
    bool InitOverlay();
    bool InitOutputMixer();
    bool InitDpTxMultirate();
    bool IsDpTxMultirateEnabled() const { return _dpTxMultirateEnabled; }    

    void WatchInput();
    bool WatchOutputTask();
    bool WatchOutputDp();
    
    void ReprogramPipePreWarp(bool resetClipper);
    void ReprogramPipePostWarp();
    void ReprogramPipePostMixer();

    bool ValidateVideoStandard(const VideoStandard& vs);

    std::string InputToString(const PipelineInput& input) const;

private:
    std::shared_ptr<VideoPipeline::PipelineCore> _crsCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _cscCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _tpgCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _vfrInputCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _rgbSwitchCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _bayerSwitchCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _usmCore;
    
    std::shared_ptr<VideoPipeline::PipelineCore> _tmoCore;

    // HDR related
    std::shared_ptr<VideoPipeline::PipelineCore> _lut1dLinearSrgbCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _lut3dSrgbHlgCore;
    //////////////

    std::shared_ptr<VideoPipeline::PipelineCore> _lut3dCore;

    std::shared_ptr<VideoPipeline::PipelineCore> _lut1dCore;

    std::shared_ptr<VideoPipeline::PipelineCore> _outputMixerTpgCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _outputMixerCore;

    std::shared_ptr<VideoPipeline::PipelineCore> _outputPipConvCore;

    std::shared_ptr<AtUtils::MessageQueue> _watchOutputQueue;

    /// The delay in milliseconds between polls to WatchOutputTask.
    static constexpr uint32_t _WATCH_OUTPUT_INTERVAL_MS = 500;

    // Pipeline input state
    std::shared_ptr<PipelineInputState> _spInputState;
    // Pipeline output state
    std::shared_ptr<PipelineOutputState> _spOutputState;

    std::function<void(const ImageConfig&)> _updateInputConfigCB;
    std::function<void(const ImageConfig&)> _updateOutputConfigCB;

    std::function<void(ImageConfig&)> _warpInputSettingCB;
    std::function<void(ImageConfig&)> _mixerInputSettingCB;
    std::function<void(ImageConfig&)> _postPipelineProgramUiUpdateCB;

    std::shared_ptr<Hapi::IHapi> _spHapi;

    // Rename to _numberOfMipis ? Use OCS instead or PIO to discover?
    enum BoardType
    {
        AlteraPremium = 0,
        AlteraModular = 1,
        Axe5Eagle = 2,
        MacnicaSulfur = 3,
        TerasicDE25 = 4,
        TerasicDE25Nano = 5,
    };
    
    BoardType _boardId = AlteraPremium;
    uint32_t _numberOfSensors = 2;
    uint32_t _framerate = 3000;

    std::deque<std::shared_ptr<VideoPipeline::PipelineCore>> _postCoreIspInputStack;
    std::shared_ptr<VideoPipeline::PipelineCore> _terminalCore;

    std::vector<ICameraPtr> _cameras;
    
    std::vector<std::shared_ptr<VideoPipeline::PipelineCore>> _sensorIntfCore;
    std::vector<Hapi::VvpExposureFusionPtr> _spInputExposureFusion;
    std::vector<std::shared_ptr<VideoPipeline::PipelineCore>> _exposureFusionCore;
    std::vector<Hapi::VvpSnoopPtr> _spInputSnoop;

    std::vector<Hapi::VvpPipConvPtr> _spInputPipConv;
    std::vector<std::shared_ptr<VideoPipeline::PipelineCore>> _inputPipConvCore;

    std::shared_ptr<SwApi::ICrs> _spCrs;
    std::shared_ptr<SwApi::ICsc> _spCsc;
    std::shared_ptr<SwApi::ITpg> _spTpg;
    std::shared_ptr<SwApi::IspVfrInput> _spVfrInput;
    std::shared_ptr<SwApi::ISwitch> _spRgbSwitch;
    
    std::shared_ptr<SwApi::ISwitch> _spBayerSwitch;

    std::vector<std::shared_ptr<CoreIspPipeline>> _coreIspPipelines;
    std::shared_ptr<Hapi::VvpUsm> _spUsm;
    std::shared_ptr<SwApi::ITmoBase> _spTmoBase;
    Hapi::Vvp3dLutPtr _spVvp3dLut;
    std::shared_ptr<SwApi::ILut3d> _spLut3d;

    std::shared_ptr<SwApi::Lut1D> _spLut1d;

    // HDR related
    std::shared_ptr<SwApi::Lut1D> _spLut1dLinearSrgb;
    std::shared_ptr<SwApi::ILut3d> _spLut3dSrgbHlg;

    // Capture frame writers
    std::shared_ptr<SwApi::VideoFrameWriter> _spVfwCaptureRaw;
    std::shared_ptr<SwApi::VideoFrameWriter> _spVfwCaptureIsp;
    
    Hapi::VvpPipConvPtr _spOutputPipConv;

    Hapi::VvpProtocolConverterPtr _spProtocolConverter;

    // Overlay related
    std::shared_ptr<IOverlayHelper> _overlayHelper;
    std::shared_ptr<SwApi::Scaler> _spOverlayScaler;
    uint32_t _overlay_mixer_width = 0;
    uint32_t _overlay_mixer_height = 0;
    bool _overlay_available = false;
    std::shared_ptr<lvglLogoHelper> _lvglLogoHelper;
 
    Hapi::VvpCpmPtr _spVvpCpm;

    std::shared_ptr<SwApi::ITpg> _spOutputMixerTpg;
    std::shared_ptr<IspMixerHelper> _spOutputMixer;

    std::vector<PipelineInput> _inputs;

    std::unique_ptr<SwApi::Remosaic> _spRemosaic;

    OutputSource _outputSrouce = OutputSource::Invalid;

    // DP multirate
    bool _dpTxMultirateEnabled;
    Hapi::VvpProtocolConverterPtr _spProtoConvLTFDpTx;
    std::shared_ptr<VideoPipeline::PipelineCore> _protoConvLTFDpTxCore;

    Hapi::PioPtr _dpTxActivDimPio;
    Hapi::PioPtr _dpTxOverrideOptsPio;
    Hapi::PioPtr _dpTxOverridePio;
    Hapi::PioPtr _dpTxStatusPio;
    bool _dpTxOverrideActive;

    std::recursive_mutex _mutex;
};
