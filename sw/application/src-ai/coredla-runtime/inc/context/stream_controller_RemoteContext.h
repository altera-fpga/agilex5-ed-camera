/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <stream_controller_IRemoteContext.h>

class StreamControllerRemoteContext : public ov::RemoteContext {
 public:
  StreamControllerRemoteContext() {};
  virtual ~StreamControllerRemoteContext() {};
  StreamControllerRemoteContext(const StreamControllerRemoteContext&) = default;
  StreamControllerRemoteContext(StreamControllerRemoteContext&&) noexcept = default;
  StreamControllerRemoteContext& operator=(const StreamControllerRemoteContext&) = default;
  StreamControllerRemoteContext& operator=(StreamControllerRemoteContext&& other) noexcept = default;

  bool UserMessage(uint32_t userMessageType, void* pPayload = nullptr, size_t size = 0, UserPayload* pUserPayloadResults = nullptr)
  {
    bool rc = false;
    auto sc_icontext = std::dynamic_pointer_cast<StreamControllerIRemoteContext>(_impl);
    if(sc_icontext)
    {
      rc = sc_icontext->UserMessage(userMessageType, pPayload, size, pUserPayloadResults);
    }
    return rc;
  }
};
