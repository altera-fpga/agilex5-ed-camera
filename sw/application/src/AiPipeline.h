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

// Interfaces
#include "FrameQueue.h"
#include "ICoreDLAIntf.h"
#include "ICoreDlaRuntime.h"
#include "AiResultsRenderer.h"
#include "HapiVvpCpm.h"

#include "VideoStandard.h"
#include "PipelineCore.h"
#include "drmHelper.h"
#include "lvglLogoHelper.h"

#include "CoreIspPipeline.h"
#include "UdxVvpIspPipeline.h"

#include <stack>
#include <map>

class AiPipeline
{
public:
    AiPipeline(std::shared_ptr<UdxVvpIspPipeline> spUdxVvpIspPipeline, std::shared_ptr<Hapi::IHapi> spHapi);

    virtual ~AiPipeline();

    bool Start();
    void Stop();

    // CoreDLA related
    std::shared_ptr<SwApi::ICoreDlaRuntime> GetCoreDlaRuntime() { return std::dynamic_pointer_cast<SwApi::ICoreDlaRuntime>(_spCoreDLAProcessor); }    
    std::shared_ptr<SwApi::IAiResultsRenderer> GetAiResultsRenderer() { return std::dynamic_pointer_cast<SwApi::IAiResultsRenderer>(_spAiResultsRenderer); }
  
private:
    bool InitCoreDLA();
    bool InitFrameQueue();


    std::shared_ptr<UdxVvpIspPipeline> _spUdxVvpIspPipeline;

    std::shared_ptr<Hapi::IHapi> _spHapi;
    std::shared_ptr<SwApi::FrameQueue> _spFrameQueue;
    std::shared_ptr<SwApi::ICoreDLAIntf> _spCoreDLAProcessor;
    std::shared_ptr<SwApi::AiResultsRenderer> _spAiResultsRenderer;
    Hapi::VvpCpmPtr _spVvpFrameQueueCpm;
 
    std::shared_ptr<VideoPipeline::PipelineCore> _frameQueueCore;
};
