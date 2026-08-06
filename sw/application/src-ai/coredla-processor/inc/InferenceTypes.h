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

#include "Logging.h"

// Logging.h defines conflict with openvino headers, so we need to undef them before including openvino headers and redefine them after.
#undef TRACE
#undef INFO
#undef WARN
#undef ERR
#undef FATAL

// Suppress warnings from OpenVINO headers
#define IN_OV_COMPONENT
#include <openvino/openvino.hpp>

#define TRACE Logging::LogTraceImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
//#define TRACE Logging::LogNothingImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define INFO Logging::LogInfoImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define WARN Logging::LogWarnImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define ERR Logging::LogErrorImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define FATAL Logging::LogFatalImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)

enum class NetworkType : uint32_t
{
    YOLOV8N,
    YOLOV8N_POSE,
    UNKNOWN
};

typedef struct {
    uint32_t _network_handle;
    NetworkType _network_type;
    std::weak_ptr<struct _Model> _model;
    ov::InferRequest _ovInferRequest;
    uint32_t _width;
    uint32_t _height;
    uint32_t _width_lt;
    uint32_t _height_lt;
    uint32_t _offset_lt;
  } InferenceRequest;

typedef struct _Model {
    uint32_t _network_handle;
    std::string _network_name;
    NetworkType _network_type;
    ov::CompiledModel _ovCompiledModel;
    std::list<InferenceRequest> _model_infer_requests;
} Model;

typedef std::shared_ptr<Model> ModelPtr;