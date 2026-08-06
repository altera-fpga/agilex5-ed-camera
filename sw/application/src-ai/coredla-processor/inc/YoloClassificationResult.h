/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __YoloClassificationResult_h__
#define __YoloClassificationResult_h__

#include <vector>
#include <cstdint>
#include "InferenceTypes.h"
#include "IResultsCallback.h"
#include "YoloOutputBlob.h"
#include "YoloParams.h"
#include "YoloDetectionObject.h"
#include "YoloClassificationItem.h"
#include "YoloNames.h"


class YoloClassificationResult
{
public:
    YoloClassificationResult(uint32_t network_handle, NetworkType network_type, uint32_t inference_count, uint32_t inference_buffer);
    virtual ~YoloClassificationResult() {}

    const std::vector<YoloClassificationItem>& GetItems() const;

    uint32_t GetNetworkHandle() const;
    NetworkType GetNetworkType() const;
    uint32_t GetInferenceCount() const;
    uint32_t GetInferenceBuffer() const;

protected:
    friend class YoloDetectionProcessor;
    friend class YoloPoseProcessor;
    // Reference
    uint32_t _network_handle;
    NetworkType _network_type;
    uint32_t _inference_count;
    uint32_t _inference_buffer;

    std::vector<YoloClassificationItem> _items;
};

#endif //__YoloClassificationResult_h__
