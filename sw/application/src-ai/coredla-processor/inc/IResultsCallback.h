/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __IRESULTSCALLBACK_H__
#define __IRESULTSCALLBACK_H__
#include <memory>
#include <string>
#include <map>

#include "InferenceTypes.h"

typedef struct {
    uint32_t network_handle;
    NetworkType network_type;
    std::vector<ov::Tensor> output_tensors;
    uint32_t inference_count;
    uint32_t inference_buffer;
}tCoreDLAOutput;


// Callback interface that you implement to receive the results
class IResultsCallback : public std::enable_shared_from_this<IResultsCallback>
{
public:
    virtual ~IResultsCallback() {}
    // These functions are called in a worker thread
    virtual void ReceiveResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput) = 0;

    // If no networks were found to load
    virtual void NoNetworkFound() = 0;

    // If The FPGA AI Suite is unlicesed there is a limited number of inferences possible
    virtual void OutOfInferences() = 0;

    // The engine generates various status messages, which are
    // sent through this interface so you can choose to printf
    // or ignore them
    virtual void StatusMessage(const char* message) = 0;
};

#endif //__IRESULTSCALLBACK_H__
