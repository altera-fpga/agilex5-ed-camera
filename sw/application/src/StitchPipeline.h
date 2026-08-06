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

#include "VideoStandard.h"
#include "PipelineCore.h"

#include "CoreIspPipeline.h"
#include "UdxVvpIspPipeline.h"

// Interfaces
#include "ITpg.h"
#include "StitchAdapter.h"
#include "MultiChannelAutoExposure.h"
#include "MultiChannelWhiteBalance.h"
#include "IMixer.h"

#include <stack>
#include <map>


class StitchPipeline
{
public:
    StitchPipeline(std::shared_ptr<UdxVvpIspPipeline> spUdxVvpIspPipeline, std::shared_ptr<Hapi::IHapi> spHapi);

    virtual ~StitchPipeline();

    bool Start();
    void Stop();

    bool InitSensorProfiling();

    // Stitch
    const std::shared_ptr<SwApi::VideoFrameReader>& GetStitchVfr() {return _spStitchVfr; }
    const Hapi::VvpAlphaChannelGeneratorPtr& GetStitchAlphaChannelGen() {return _spAlphaChGen; }
    const std::shared_ptr<SwApi::StitchAdapter>& GetStitchAdapter() { return _spStitchAdapter; }
    const std::shared_ptr<SwApi::MultiChannelAutoExposure>& GetMultiChannelAutoExposure() { return _spMultiChannelAutoExposure; }
    const std::shared_ptr<SwApi::MultiChannelWhiteBalance>& GetMultiChannelWhiteBalance() { return _spMultiChannelWhiteBalance; }

    void LinkPipelineCores();

private:
    bool InitSitchAlpha();
    bool InitStitch();
    bool InitStitchMixerTpg();
    bool InitStitchMixer();

    std::shared_ptr<UdxVvpIspPipeline> _spUdxVvpIspPipeline;
    std::shared_ptr<Hapi::IHapi> _spHapi;

    std::stack<std::shared_ptr<VideoPipeline::PipelineCore>> _stitchStack;

    std::vector<std::shared_ptr<VideoPipeline::PipelineCore>> _stitchInputCore;
    std::shared_ptr<VideoPipeline::PipelineCore> _stitchMixerTpgCore;
    std::vector<std::shared_ptr<VideoPipeline::PipelineCore>> _stitchMixerInputCores;
    std::shared_ptr<VideoPipeline::PipelineCore> _stitchMixerOutputCore;

    // Stitch
    std::shared_ptr<SwApi::VideoFrameReader> _spStitchVfr;
    Hapi::VvpAlphaChannelGeneratorPtr _spAlphaChGen;
    std::shared_ptr<SwApi::StitchAdapter> _spStitchAdapter;
    std::shared_ptr<SwApi::MultiChannelAutoExposure> _spMultiChannelAutoExposure;
    std::shared_ptr<SwApi::MultiChannelWhiteBalance> _spMultiChannelWhiteBalance;
    std::shared_ptr<SwApi::IMixer> _spStitchMixer;
    std::shared_ptr<SwApi::ITpg> _spStitchMixerTpg;
};
