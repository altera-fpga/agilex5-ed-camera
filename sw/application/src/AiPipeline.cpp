/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "UdxVvpIspPipeline.h"
#include "AiPipeline.h"
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
#include "IspCommon.h"
#include "HapiCoreDLA.h"
#include "VvpIspDemo.h"
#include "CommonUiUpdate.h"

using namespace SwApi;
using namespace VideoPipeline;

AiPipeline::AiPipeline(std::shared_ptr<UdxVvpIspPipeline> spUdxVvpIspPipeline, std::shared_ptr<Hapi::IHapi> spHapi)
    : _spUdxVvpIspPipeline(spUdxVvpIspPipeline), _spHapi(spHapi)
{
    bool coreDLAFound = InitCoreDLA();
    bool frameQueueFound = InitFrameQueue();

    if (!frameQueueFound || !coreDLAFound)
    {
        ERR << "AI subsystem not detected\n";
    }
}

AiPipeline::~AiPipeline()
{
}


bool AiPipeline::Start()
{
    return true;
}

void AiPipeline::Stop()
{
    if(_spCoreDLAProcessor)
        _spCoreDLAProcessor->Stop();
}

bool AiPipeline::InitCoreDLA()
{
    // Hapi initialisation for CoreDLA here
    _spCoreDLAProcessor = SwApi::ICoreDLAIntf::Create(_spHapi);
    return true;

}

bool AiPipeline::InitFrameQueue()
{

    if (!(_spCoreDLAProcessor))
    {
        return false;
    }

    _spFrameQueue = SwApi::FrameQueue::Create(_spCoreDLAProcessor);
    _spAiResultsRenderer = AiResultsRenderer::Create(_spCoreDLAProcessor);

    // Must create frame queue before loading network (stream controller ping will fail in S2M mode)
    _spCoreDLAProcessor->LoadNetwork();

    // Must load network before initialising frame queue
    _spFrameQueue->Initialise(3840, 2160);
    _spAiResultsRenderer->Initialise();

    // Start inference after frame queue is initialised
    _spCoreDLAProcessor->Start();

    // Pipeline core
    auto frameQueuePipelineCoreInputHandler =
        [this](const ImageConfig& inputDetails) -> ImageConfig 
        {
            ASSERT(_frameQueueCore->_inputCores.size() == 1 && _frameQueueCore->_outputCores.size() == 1);

            TRACE << "[AI Camera Frame Queue]: " << LOG_V(inputDetails) << '\n';

            _frameQueueCore->_currentInputVideoDetails = inputDetails;
            _frameQueueCore->_outputCores[0]->ApplyInputVideoInfo(inputDetails);

            return inputDetails;
        };
    auto frameQueuePipelineCoreOutputHandler =
        [this](const ImageConfig& outputDetails) -> ImageConfig 
        {
            ASSERT(_frameQueueCore->_inputCores.size() == 1 && _frameQueueCore->_outputCores.size() == 1);

            TRACE << "[AI Camera Frame Queue]: " << LOG_V(outputDetails) << '\n';

            _spFrameQueue->SetVideoResolution(outputDetails._width, outputDetails._height);

            _frameQueueCore->_currentOutputVideoDetails = outputDetails;
            _frameQueueCore->_inputCores[0]->ApplyOutputVideoInfo(outputDetails);

            return outputDetails;
        };
    _frameQueueCore = PipelineCore::Create(_spFrameQueue, frameQueuePipelineCoreInputHandler, frameQueuePipelineCoreOutputHandler);

    _spUdxVvpIspPipeline->GetPostCoreIspInputStack().push_front(_frameQueueCore);

    return true;
}
