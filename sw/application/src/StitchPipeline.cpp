/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "StitchPipeline.h"
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
#include "CompileTimeConfiguration.h"
#include "IspCommon.h"
#include "Logging.h"
#include "intel_vvp_core.h"
#include "VvpIspDemo.h"
#include "CommonUiUpdate.h"


using namespace SwApi;
using namespace VideoPipeline;

StitchPipeline::StitchPipeline(std::shared_ptr<UdxVvpIspPipeline> spUdxVvpIspPipeline, std::shared_ptr<Hapi::IHapi> spHapi):
    _spUdxVvpIspPipeline(spUdxVvpIspPipeline),
    _spHapi(spHapi)
{
    bool stitchMixerTpgFound = InitStitchMixerTpg();
    if (!stitchMixerTpgFound)
    {
        ERR << "Failed to initialise Stitch TPG Mixer.\n";
    }

    bool stitchAlphaFound = InitSitchAlpha();
    if (!stitchAlphaFound)
    {
        ERR << "Failed to initialise Stitch Alpha.\n";
    }

    bool stitchMixerFound = InitStitchMixer();
    if (!stitchMixerFound)
    {
        ERR << "Failed to initialise Stitch Mixer.\n";
    }

    bool stitchInitialised = InitStitch();
    if (!stitchInitialised)
    {
        ERR << "Failed to initialise stitch subsystem.\n";
    }

    bool profileInitialised = InitSensorProfiling();
    if (!profileInitialised)
    {
        ERR << "Failed to initialise sensor profile subsystem.\n";
    }
}


StitchPipeline::~StitchPipeline()
{
}

bool StitchPipeline::Start()
{
    return true;
}

void StitchPipeline::Stop()
{
}

bool StitchPipeline::InitSitchAlpha()
{
    const uint32_t STITCH_ALPHA_CH_GEN_UID = UdxVvpIspPipeline::PipelineBaseUID(UdxVvpIspPipeline::PipelineSubsystem::CoreISP, 1);
    const uint32_t STITCH_VFR_UID = UdxVvpIspPipeline::PipelineBaseUID(UdxVvpIspPipeline::PipelineSubsystem::CoreISP, 1);

    _spAlphaChGen = _spHapi->CreateByUniqueID<Hapi::VvpAlphaChannelGenerator>(STITCH_ALPHA_CH_GEN_UID);
    if(_spAlphaChGen)
    {
        TRACE << "Found Stitch Alpha Channel Generator\n";
    }
    else
    {
        WARN << "Stitch Alpha Channel Generator not available\n";
        auto spStitchVfrInst = _spHapi->CreateByUniqueID<Hapi::VvpVfr>(STITCH_VFR_UID);

        if(!spStitchVfrInst)
        {
            WARN << "Stitch Video Frame Reader not available\n";
            return false;
        }

        ::intel_vvp_core_set_img_info_interlace(spStitchVfrInst->GetInstance(), 0);
        ::intel_vvp_core_set_img_info_subsampling(spStitchVfrInst->GetInstance(), 0x3);
        ::intel_vvp_core_set_img_info_cositing(spStitchVfrInst->GetInstance(), 0);
        ::intel_vvp_core_set_img_info_colorspace(spStitchVfrInst->GetInstance(), 0);

        static const uintptr_t BUFFER_ADDR_FPGA = 0x20000000;
        static const uintptr_t BUFFER_ADDR_CPU  = 0x80000000;

        _spStitchVfr = std::make_shared<VideoFrameReader>(spStitchVfrInst, BUFFER_ADDR_CPU, BUFFER_ADDR_FPGA);

        if (!_spStitchVfr)
            return false;

        _spStitchVfr->SetResolutionOfImage(1920, 1080);
    }

    return true;
}

bool StitchPipeline::InitStitch()
{
    _stitchInputCore.resize(_spUdxVvpIspPipeline->GetNumberCoreIspPipelines());
    for(uint32_t pipeline_index = 0; pipeline_index < _spUdxVvpIspPipeline->GetNumberCoreIspPipelines(); pipeline_index++)
    {
        auto coreIspPipeline = _spUdxVvpIspPipeline->GetCoreIspPipeline(pipeline_index);
        // Stitch Input Pipeline Core
        auto stitchInputCorePipelineInputHandler =
            [this, &coreIspPipeline, pipeline_index](const ImageConfig& inputDetails) -> ImageConfig
            {
                ASSERT(_stitchInputCore[pipeline_index]->_inputCores.size() == 1,
                        "The Stitch input core must take have 1 input.");

                TRACE << "[Stitch Input " << pipeline_index << "]:\t" << LOG_V(inputDetails) << '\n' << std::flush;
                if(_stitchInputCore[pipeline_index]->_currentInputVideoDetails != inputDetails)
                {
                    _stitchInputCore[pipeline_index]->_currentInputVideoDetails;
                    if(_stitchInputCore[pipeline_index]->_currentInputVideoDetails.IsValid())
                    {
                        if(_spStitchMixer)
                        {
                            _spStitchMixer->SetLayerEnableState(1+pipeline_index, SwApi::MixerLayerState::MixerLayerEnabledSoftStart);
                        }
                    }
                    else
                    {
                        if(_spStitchMixer)
                        {
                            _spStitchMixer->SetLayerEnableState(1+pipeline_index, SwApi::MixerLayerState::MixerLayerConsumedEnabledSoftStart);
                        }
                    }
                }

                return inputDetails;
            };
        auto stitchInputCorePipelineOutputHandler =
            [this, &coreIspPipeline, pipeline_index](const ImageConfig& outputDetails) -> ImageConfig
            {
                ASSERT(_stitchInputCore[pipeline_index]->_inputCores.size() == 1,
                        "The Stitch input core must take have 1 input.");

                TRACE << "[Stitch Input " << pipeline_index << "]:\t" << LOG_V(outputDetails) << '\n' << std::flush;
                if(_stitchInputCore[pipeline_index]->_inputCores[0]->_currentOutputVideoDetails != outputDetails)
                {
                    _stitchInputCore[pipeline_index]->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);
                    _spStitchAdapter->SetStitchWarpResolution(pipeline_index, outputDetails.Width(), outputDetails.Height());
                }

                return outputDetails;
            };
        _stitchInputCore[pipeline_index] = PipelineCore::Create(_spStitchAdapter, stitchInputCorePipelineInputHandler, stitchInputCorePipelineOutputHandler);
    }
    std::vector<std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig>> vspStitchInputConfig;
    for(uint32_t pipeline_index = 0; pipeline_index < _spUdxVvpIspPipeline->GetNumberCoreIspPipelines(); pipeline_index++)
    {
        auto coreIspPipeline = _spUdxVvpIspPipeline->GetCoreIspPipeline(pipeline_index);
        std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig> stitchInputConfig = std::make_shared<SwApi::StitchAdapter::tStitchInputConfig>();
        if(pipeline_index == 1)
        {
            stitchInputConfig->_spStitchVfr = _spStitchVfr;
            stitchInputConfig->_spAlphaChGen = _spAlphaChGen;
        }

        stitchInputConfig->_inputCore = _stitchInputCore[pipeline_index];
        stitchInputConfig->_spWarpAdapter = coreIspPipeline->GetWarpAdapter();

        vspStitchInputConfig.emplace_back(stitchInputConfig);
    }
    _spStitchAdapter = SwApi::StitchAdapter::Create(vspStitchInputConfig, _spStitchMixer);
    for(uint32_t pipeline_index = 0; pipeline_index < _spUdxVvpIspPipeline->GetNumberCoreIspPipelines(); pipeline_index++)
    {
        auto coreIspPipeline = _spUdxVvpIspPipeline->GetCoreIspPipeline(pipeline_index);
        auto parentApplyInputHandler = coreIspPipeline->GetInputCore()->_applyInputHandler;
        coreIspPipeline->GetInputCore()->_applyInputHandler =
            [this, parentApplyInputHandler = std::move(parentApplyInputHandler), pipeline_index](const ImageConfig& inputConfig) -> ImageConfig
            {
                if(_spStitchAdapter)
                {
                    TRACE << "[Stitch Sensor " << pipeline_index << "]:\t" << LOG_V(inputConfig) << '\n' << std::flush;
                    _spStitchAdapter->SetStitchSensorResolution(pipeline_index, inputConfig.Width(), inputConfig.Height());
                }
                return parentApplyInputHandler(inputConfig);
             };
    }
    return true;
}

bool StitchPipeline::InitStitchMixerTpg()
{
    // TODO : Move these IDs to configuration provider class.
    // (reads from default -> JSON -> command line)
    const uint32_t MIXER_TPG_UID = UdxVvpIspPipeline::PipelineBaseUID(UdxVvpIspPipeline::PipelineSubsystem::PostCoreISP);

    auto spVvpMixerTpgInst = _spHapi->CreateByUniqueID<Hapi::VvpTpg>(MIXER_TPG_UID);
    if (!spVvpMixerTpgInst)
    {
        return false;
    }

    _spStitchMixerTpg = ITpg::Create(spVvpMixerTpgInst, 1920, 1080);
    if (!_spStitchMixerTpg)
    {
        return false;
    }

    _spStitchMixerTpg->SetOutputWidth(3840);
    _spStitchMixerTpg->SetOutputHeight(2160);
#ifdef MIXER_DEBUG
    _spStitchMixerTpg->SetColorsForUniformPattern(0x100, 0x0, 0x0);
#else
    _spStitchMixerTpg->SetColorsForUniformPattern(0x0, 0x0, 0x0);
#endif
	// The Stitch Mixer TPG needs to be activated last
    _spStitchMixerTpg->SetEnable(true);

    // Pipeline core

    auto stitchMixerTpgCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            ASSERT(_stitchMixerTpgCore->_outputCores.size() == 1);
            ASSERT(_stitchMixerTpgCore->_inputCores.size() == 0, "The VVP TPG does not take input - it's an output-only core.");

            _spStitchMixerTpg->SetOutputWidth(outputDetails._width);
            _spStitchMixerTpg->SetOutputHeight(outputDetails._height);

            TRACE << "[Stitch Mixer TPG]: " << LOG_V(outputDetails) << "\n" << std::flush;

            _stitchMixerTpgCore->_currentOutputVideoDetails = outputDetails;

            return outputDetails;
        };
    _stitchMixerTpgCore = PipelineCore::Create(_spStitchMixerTpg, nullptr, stitchMixerTpgCorePipelineOutputHandler);

    return true;
}

bool StitchPipeline::InitStitchMixer()
{
    const uint32_t MIXER_UID = UdxVvpIspPipeline::PipelineBaseUID(UdxVvpIspPipeline::PipelineSubsystem::PostCoreISP);
    auto spVvpMixerInst = _spHapi->CreateByUniqueID<Hapi::VvpMixer>(MIXER_UID);
    if (!spVvpMixerInst) return false;

    _spStitchMixer = IMixer::Create(spVvpMixerInst);
    if (!_spStitchMixer) return false;

    ::intel_vvp_core_set_img_info_subsampling(spVvpMixerInst->GetInstance(), 0x3);

    uint32_t num_inputs = _spStitchMixer->GetMaxLayersSupported();
    _stitchMixerInputCores.resize(num_inputs);
    for(uint32_t stitchMixerInput = 0; stitchMixerInput < num_inputs; stitchMixerInput++)
    {
        auto stitchMixerCoreInputPipelineHandler =
            [this, stitchMixerInput](const ImageConfig& inputDetails) -> ImageConfig
            {
                // The input handler only corresponds to the Pipeline layer, and has no output

                TRACE << "[Stitch Mixer Input " << stitchMixerInput << "]: " << LOG_V(inputDetails) << '\n' << std::flush;

                _stitchMixerInputCores[stitchMixerInput]->_currentInputVideoDetails = inputDetails;
                for(auto& inputCore :_stitchMixerInputCores[stitchMixerInput]->_inputCores)
                {
                    inputCore->_currentOutputVideoDetails = inputDetails;
                    inputCore->ApplyInputVideoInfo(inputCore->_currentInputVideoDetails);
                }

                return inputDetails;
            };

        _stitchMixerInputCores[stitchMixerInput] = PipelineCore::Create(_spStitchMixer, stitchMixerCoreInputPipelineHandler);
    }

    auto stitchMixerCorePipelineOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig
        {
            ASSERT(_stitchMixerOutputCore->_outputCores.size() == 1);

            _stitchMixerOutputCore->_currentInputVideoDetails = outputDetails;
            _stitchMixerOutputCore->_currentOutputVideoDetails = outputDetails;

            TRACE << "[Stitch Mixer Output]: " << LOG_V(outputDetails) << '\n' << std::flush;

            if(outputDetails.IsValid())
            {
                if(_spStitchMixer) 
                {
                    _spStitchMixer->SetOutputWidth(outputDetails._width);
                    _spStitchMixer->SetOutputHeight(outputDetails._height);
                }

                if(_spStitchAdapter)
                {
                    _spStitchAdapter->SetStitchOutputResolution(outputDetails._width, outputDetails._height);
                }
            }

            // The mixer needs to feed this back to the TPG to keep its resolution set
            if (_stitchMixerOutputCore->_inputCores.size() == 1)
            {
                _stitchMixerOutputCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);
            }

            return outputDetails;
        };
        
    _stitchMixerOutputCore = PipelineCore::Create(_spStitchMixer, nullptr, stitchMixerCorePipelineOutputHandler);
    _spUdxVvpIspPipeline->GetPostCoreIspInputStack().push_front(_stitchMixerOutputCore);

    for(uint32_t pipeline = 0; pipeline < _spUdxVvpIspPipeline->GetNumberCoreIspPipelines(); pipeline++)
    {
        SwApi::MixerBlendMode blend_mode = SwApi::MixerBlendMode::MixerBlendModeOpaque;
        if(pipeline > 0)
        {
            blend_mode = SwApi::MixerBlendMode::MixerBlendModeInputAlpha;
        }
        _spStitchMixer->SetLayerBlendMode(pipeline + 1U, blend_mode);
        _spStitchMixer->SetLayerEnableState(pipeline + 1U, SwApi::MixerLayerState::MixerLayerEnabledSoftStart);
    }

    return true;
}

void StitchPipeline::LinkPipelineCores()
{
    for(uint32_t pipeline_index = 0; pipeline_index < _spUdxVvpIspPipeline->GetNumberCoreIspPipelines(); pipeline_index++)
    {
        auto coreIspPipeline = _spUdxVvpIspPipeline->GetCoreIspPipeline(pipeline_index);
        _stitchInputCore[pipeline_index]->ConnectInput(coreIspPipeline->GetOutputCore());
    }

    _stitchMixerOutputCore->ConnectInput(_stitchMixerTpgCore);
}

bool StitchPipeline::InitSensorProfiling()
{
    std::vector<std::shared_ptr<WhiteBalanceController>> vspWhiteBalance;
    std::vector<std::shared_ptr<AutoExposureController>> vspAutoExposure;

    for(uint32_t pipeline_index = 0; pipeline_index < _spUdxVvpIspPipeline->GetNumberCoreIspPipelines(); pipeline_index++)
    {
        auto coreIspPipeline = _spUdxVvpIspPipeline->GetCoreIspPipeline(pipeline_index);
        vspWhiteBalance.emplace_back(coreIspPipeline->GetWhiteBalanceController());
        vspAutoExposure.emplace_back(coreIspPipeline->GetAutoExposureController());
    }

    _spMultiChannelWhiteBalance = MultiChannelWhiteBalance::Create();
    if(_spMultiChannelWhiteBalance)
    {
        _spMultiChannelWhiteBalance->SetWhiteBalanceControllers(vspWhiteBalance);
    }

    _spMultiChannelAutoExposure = MultiChannelAutoExposure::Create();
    if(_spMultiChannelAutoExposure)
    {
        _spMultiChannelAutoExposure->SetAutoExposureControllers(vspAutoExposure);
    }

    return true;
}