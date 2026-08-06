/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

// OpenVINO plugin developer api
#include "openvino/runtime/iplugin.hpp"
#include <stream_controller_userMessages.h>

class StreamControllerIRemoteContext : public ov::IRemoteContext {
 public:
  StreamControllerIRemoteContext() {};
  virtual ~StreamControllerIRemoteContext() {};
  virtual bool UserMessage(uint32_t userMessageType, void* pPayload = nullptr, size_t size = 0, UserPayload* pUserPayloadResult = nullptr) = 0;
};

using StreamControllerIRemoteContextPtr=std::shared_ptr<StreamControllerIRemoteContext>;