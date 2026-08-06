/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "PipelineCore.h"
#include "IspCommon.h"

#include <algorithm>
#include <iostream>

namespace VideoPipeline
{

PipelineCore::PipelineCore(std::shared_ptr<void> core,
                           std::function<ImageConfig(ImageConfig)> applyInputHandler,
                           std::function<ImageConfig(ImageConfig)> applyOutputHandler)
    : _core(std::move(core))
    , _applyInputHandler(std::move(applyInputHandler))
    , _applyOutputHandler(std::move(applyOutputHandler))
{
}

std::shared_ptr<PipelineCore> PipelineCore::Create(
    std::shared_ptr<void> core, std::function<ImageConfig(ImageConfig)> applyInputHandler,
    std::function<ImageConfig(ImageConfig)> applyOutputHandler)
{
    return std::make_shared<PipelineCore>(core, applyInputHandler, applyOutputHandler);
}

ImageConfig PipelineCore::ApplyInputVideoInfo(const ImageConfig& inputVideoDetails)
{
    return _applyInputHandler != nullptr ? _applyInputHandler(inputVideoDetails) : inputVideoDetails;
}

ImageConfig PipelineCore::ApplyOutputVideoInfo(const ImageConfig& outputVideoDetails)
{
    return _applyOutputHandler != nullptr ? _applyOutputHandler(outputVideoDetails) : outputVideoDetails;
}

ImageConfig PipelineCore::GetCurrentInputVideoInfo()
{
    return _currentInputVideoDetails;
}
ImageConfig PipelineCore::GetCurrentOutputVideoInfo()
{
    return _currentOutputVideoDetails;
}

/// Returns a shared pointer to the underlying SwApi instance.
std::shared_ptr<void> PipelineCore::Unveil()
{
    return _core;
}

bool PipelineCore::ConnectInput(PipelineCore* input)
{
    if (input == nullptr or IsCoreConnectedAsOutput(input)) {
        return false;
    }

    if (IsCoreConnectedAsInput(input)) {
        return true;
    }

    _inputCores.push_back(input);
    input->_outputCores.push_back(this);
    return true;
}

bool PipelineCore::ConnectInput(std::shared_ptr<PipelineCore> input) {
    return ConnectInput(input.get());
}

std::vector<PipelineCore*>& PipelineCore::GetInputCores()
{
    return _inputCores;
}

bool PipelineCore::IsCoreConnectedAsInput(PipelineCore* core) {
    return std::find(_inputCores.cbegin(), _inputCores.cend(), core) != _inputCores.cend();
}

bool PipelineCore::IsCoreConnectedAsOutput(PipelineCore* core) {
    return std::find(_outputCores.cbegin(), _outputCores.cend(), core) != _outputCores.cend();
}

bool PipelineCore::ConnectOutput(PipelineCore* output)
{
    if (output == nullptr or IsCoreConnectedAsInput(output)) {
        return false;
    }

    if (IsCoreConnectedAsOutput(output)) {
        return true;
    }

    _outputCores.push_back(output);
    output->_inputCores.push_back(this);
    return true;
}

bool PipelineCore::ConnectOutput(std::shared_ptr<PipelineCore> output) {
    return ConnectOutput(output.get());
}

std::vector<PipelineCore*> PipelineCore::GetOutputCores()
{
    return _outputCores;
}

} // namespace VideoPipeline
