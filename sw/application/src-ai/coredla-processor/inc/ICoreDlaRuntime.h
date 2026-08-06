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
#include <vector>
#include "InferenceTypes.h"

namespace SwApi {
/**
 * High-level interface definition for the Core DLA runtime.
 */

struct ICoreDlaRuntime {
    virtual ~ICoreDlaRuntime() {};

    virtual const std::vector<ModelPtr>& GetNetworkList() = 0;

    virtual void SetNetworkHandle(uint32_t network_handle) = 0;
    virtual void SetDetectionThreshold(float detectionThreshold) = 0;
    virtual void SetIOUThreshold(float iouThreshold) = 0;
    virtual void SetDiagnostics(bool diagnostics) = 0;
};

}
