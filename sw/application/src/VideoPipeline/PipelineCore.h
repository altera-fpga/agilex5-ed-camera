/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <cstdint>
#include <iosfwd>
#include <functional>
#include <memory>
#include <vector>
#include "VideoStandard.h"

namespace VideoPipeline
{

struct PipelineCore 
{
    PipelineCore(std::shared_ptr<void> core,
                 std::function<ImageConfig(ImageConfig)> applyInputHandler,
                 std::function<ImageConfig(ImageConfig)> applyOutputHandler = nullptr);

    static std::shared_ptr<PipelineCore> Create(
        std::shared_ptr<void> core,
        std::function<ImageConfig(ImageConfig)> applyInputHandler,
        std::function<ImageConfig(ImageConfig)> applyOutputHandler = nullptr);

    ~PipelineCore() {};

    ImageConfig ApplyInputVideoInfo(const ImageConfig& inputVideoDetails);
    ImageConfig ApplyOutputVideoInfo(const ImageConfig& outputVideoDetails);

    ImageConfig GetCurrentInputVideoInfo();
    ImageConfig GetCurrentOutputVideoInfo();

    /// Connect the core `input` to serve as an input to `this`, passing video to the instance.
    /// ConnectInput() will update both cores' input and output lists as required..
    bool ConnectInput(PipelineCore* input);
    bool ConnectInput(std::shared_ptr<PipelineCore> input);
    std::vector<PipelineCore*>& GetInputCores();
    /// Connect the core `output` to serve as a sink to `this`, receving video from the instance.
    /// ConnectOutput() will update both cores' input and output lists as required.
    bool ConnectOutput(PipelineCore* output);
    bool ConnectOutput(std::shared_ptr<PipelineCore> input);
    std::vector<PipelineCore*> GetOutputCores();

    bool IsCoreConnectedAsInput(PipelineCore* core);
    bool IsCoreConnectedAsOutput(PipelineCore* core);

    /// Returns a shared pointer to the underlying SwApi instance.
    std::shared_ptr<void> Unveil();

    std::shared_ptr<void> _core = nullptr;
    std::function<ImageConfig(ImageConfig inputDetails)> _applyInputHandler;
    std::function<ImageConfig(ImageConfig outputDetails)> _applyOutputHandler;

    ImageConfig _currentInputVideoDetails;
    ImageConfig _currentOutputVideoDetails;

    std::vector<PipelineCore*> _inputCores;
    std::vector<PipelineCore*> _outputCores;
};

} // namespace VideoPipeline
