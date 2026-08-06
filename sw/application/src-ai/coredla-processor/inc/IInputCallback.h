/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __IINPUTCALLBACK_H__
#define __IINPUTCALLBACK_H__
#include <memory>

// interface that you implement to schedule inferences
class IInference : public std::enable_shared_from_this<IInference>
{
public:
    virtual ~IInference() {}
    virtual void Inference(float* data) = 0;
};

// Callback interface that you implement to process input buffers
class IInputCallback : public std::enable_shared_from_this<IInputCallback>
{
public:
    virtual ~IInputCallback() {}
    virtual void RegisterInferenceCallback(std::shared_ptr<IInference> inferenceCallback) = 0;
};

#endif //__IINPUTCALLBACK_H__
