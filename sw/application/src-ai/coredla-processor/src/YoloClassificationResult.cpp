/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "YoloClassificationResult.h"

YoloClassificationResult::YoloClassificationResult(uint32_t network_handle, NetworkType network_type, uint32_t inference_count, uint32_t inference_buffer)
: _network_handle(network_handle)
, _network_type(network_type)
, _inference_count(inference_count)
, _inference_buffer(inference_buffer)
{
}

const std::vector<YoloClassificationItem>& YoloClassificationResult::GetItems() const
{
    return _items;
}

uint32_t YoloClassificationResult::GetNetworkHandle() const
{
    return _network_handle;
}

NetworkType YoloClassificationResult::GetNetworkType() const
{
    return _network_type;
}

uint32_t YoloClassificationResult::GetInferenceCount() const
{
    return _inference_count;
}

uint32_t YoloClassificationResult::GetInferenceBuffer() const
{
    return _inference_buffer;
}
